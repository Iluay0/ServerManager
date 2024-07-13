#pragma once

#include "Util/Util.h"

struct Command
{
	std::string name;
	std::string description;
	std::function<void(std::deque<std::string>)> func;

	Command(const std::string& name, const std::string& description, const std::function<void(std::deque<std::string>)>& func)
		: name(name), description(description), func(func)
	{
	}

	Command() = default;
};

class InputModule
{
public:
	InputModule() {};
	~InputModule() {};

	virtual void onExit() {};

	inline bool onInput(std::string input)
	{
		std::string prefix = getName() + ":";
		if (input.starts_with(prefix))
			input.erase(input.find(prefix), prefix.length());
		else
		{
			if (!isSkipAlias())
				return false;
		}

		if (input.contains(":"))
		{
			auto split = util::splitString<std::deque<std::string>>(input, ":");
			if (getModule(split.front()))
				return getModule(split.front())->onInput(input);
		}

		for (const auto& [name, command] : m_commands)
		{
			if (input.starts_with(name))
			{
				command.func(getArgs(input));
				return true;
			}
		}

		return false;
	};

	bool isSkipAlias() { return m_skipAlias; }
	std::string getName() { return m_name; }
	std::string getDescription() { return m_description; }
	void setInterrupt(bool interrupt) { m_interrupt = interrupt; }

	void print()
	{
		std::cout << "Module " << dye::on_light_blue(getName()) << ":" << std::endl;
		std::cout << getDescription() << std::endl;
		if (isSkipAlias())
			std::cout << "The commands of this module can be used without entering its name." << std::endl;
		std::cout << std::endl;

		printCommands();
		printModules();
	}

	void printCommands()
	{
		if (m_commands.empty())
			return;

		std::cout << "Commands:" << std::endl;
		for (auto& [name, command] : m_commands)
		{
			std::cout << " - " << dye::on_light_aqua(name) << ": " << command.description << std::endl;
		}
		std::cout << std::endl;
	};

	void printModules()
	{
		if (m_modules.empty())
			return;

		std::cout << "Sub-modules:" << std::endl;
		for (auto& [name, module] : m_modules)
		{
			std::cout << " - " << dye::on_light_aqua(name) << ": " << module->getName() << std::endl;
		}
		std::cout << std::endl;
	};

	const std::map<std::string, std::shared_ptr<InputModule>>& getModules() { return m_modules; }
	const std::shared_ptr<InputModule> getModule(std::string name)
	{
		if (!m_modules.count(name))
			return nullptr;
		return m_modules[name];
	}
protected:
	void addCommand(std::string name, std::function<void(std::deque<std::string>)> func, std::string description)
	{
		m_commands[name] = Command(name, description, func);
	};
	virtual void bindCommands() {};

	void setName(std::string name) { m_name = name; }
	void setDescription(std::string description) { m_description = description; }
	void setSkipAlias(bool skipAlias) { m_skipAlias = skipAlias; }
	bool isInterrupt() { return m_interrupt; }

	void addModule(std::shared_ptr<InputModule> module) { m_modules.insert({ module->getName(), module }); }
	virtual void createChildModules() {};
private:
	inline std::deque<std::string> getArgs(std::string szInput)
	{
		std::deque<std::string> strings = util::splitString<std::deque<std::string>>(szInput, " ");
		strings.pop_front();
		return strings;
	}
private:
	std::string m_name = "";
	std::string m_description = "";
	bool m_interrupt = false;
	bool m_skipAlias = false;
	std::map<std::string, Command> m_commands;
	std::map<std::string, std::shared_ptr<InputModule>> m_modules;
};