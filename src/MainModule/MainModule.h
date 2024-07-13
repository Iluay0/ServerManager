#pragma once

#include "../InputModule.h"

class MainModule : public InputModule
{
public:
	MainModule();
	~MainModule();

	bool isRunning() { return m_running; }
	void stop() { m_running = false; }
protected:
	void bindCommands() override;
	void createChildModules() override;
private:
	void help(std::deque<std::string> args);
private:
	bool m_running = true;
};