#pragma once

#pragma warning(disable : 4996)

#include <Windows.h>
#include <signal.h>
#include <process.h>
#include <Tlhelp32.h>
#include <winbase.h>

#include <thread>
#include <chrono>
#include <thread>

#include <string>
#include <deque>
#include <set>
#include <map>
#include <vector>

#include <iostream>
#include <fstream>
#include <functional>
#include <filesystem>
#include <ranges>
#include <regex>
#include <thread>
#include <future>
#include <chrono>

#include <color.hpp>
#include <json.hpp>

using namespace std::chrono_literals;

constexpr auto configFile = ".\\config.json";
constexpr auto dbLastMigrationFile = ".\\latest_migration.txt";
constexpr auto dbMigrationsFolder = ".\\DBMigrations";
constexpr auto migrationDateFormat = "%Y-%m-%d-%H%M%S";

namespace util
{
	struct LogPoolerConfig
	{
		bool m_enabled = false;
		std::deque<std::string> m_folders;
		std::string m_regexFileName = "";
		std::string m_regexExeName = "";
	};

	class Config
	{
	public:
		Config() {}
		~Config() {}

		void load()
		{
			std::ifstream file(configFile);
			if (!file.is_open())
			{
				std::cout << dye::light_red("Could not open file: " + std::string(configFile) + ".") << std::endl;
				return;
			}

			try
			{
				auto json = nlohmann::ordered_json::parse(file);

				m_dbInstanceName = json.value("dbInstanceName", "");
				m_slashCommandsFileName = json.value("slashCommandsFile", "");

				if (json.contains("executables"))
				{
					for (const auto& [path, executables] : json["executables"].items())
					{
						for (const auto& exeName : executables)
						{
							m_exePaths.push_back({ path, exeName.get<std::string>() });
						}
					}
				}

				if (json.contains("copy"))
				{
					for (const auto& el : json["copy"])
					{
						m_copyPaths.push_back({ el["from"].get<std::string>(), el["to"].get<std::string>() });
					}
				}

				if (json.contains("logPooler"))
				{
					m_logPoolerConfig.m_enabled = json["logPooler"]["enabled"].get<bool>();
					for (const auto& el : json["logPooler"]["folders"])
					{
						m_logPoolerConfig.m_folders.push_back(el.get<std::string>());
					}
					m_logPoolerConfig.m_regexFileName = json["logPooler"]["regexFileName"].get<std::string>();
					m_logPoolerConfig.m_regexExeName = json["logPooler"]["regexExeName"].get<std::string>();
				}
			}
			catch (nlohmann::json::exception e)
			{
				std::cout << dye::light_red("Could not parse file: " + std::string(configFile) + ".") << std::endl;
			}
		}

		LogPoolerConfig& getLogPoolerConfig() { return m_logPoolerConfig; }
		std::string getDbInstanceName() { return m_dbInstanceName; }
		std::string getSlashCommandsFileName() { return m_slashCommandsFileName; }
		const std::deque<std::pair<std::string, std::string>>& getExePaths() { return m_exePaths; }
		const std::deque<std::pair<std::string, std::string>>& getCopyPaths() { return m_copyPaths; }

		static Config& inst()
		{
			static Config instance;
			return instance;
		}
	private:
		std::string m_dbInstanceName = "";
		std::string m_slashCommandsFileName = "";
		std::deque<std::pair<std::string, std::string>> m_exePaths;
		std::deque<std::pair<std::string, std::string>> m_copyPaths;
		LogPoolerConfig m_logPoolerConfig;
	};

	template <typename T>
	inline T splitString(std::string str, std::string delimiter)
	{
		T strings{};

		std::string::size_type pos = 0;
		std::string::size_type prev = 0;
		while ((pos = str.find(delimiter, prev)) != std::string::npos)
		{
			strings.push_back(str.substr(prev, pos - prev));
			prev = pos + delimiter.size();
		}
		strings.push_back(str.substr(prev));

		return strings;
	}

	inline time_t getUnixTime(std::string date, std::string format)
	{
		std::chrono::sys_seconds tp;
		std::stringstream ssTimestamp(date);
		ssTimestamp >> parse(format.c_str(), tp);
		return std::chrono::system_clock::to_time_t(tp);
	}

	inline std::string getTimeString(time_t time, std::string format)
	{
		std::tm* now = std::gmtime(&time);

		std::stringstream buffer;
		buffer << std::put_time(now, format.c_str());

		return buffer.str();
	}

	inline std::set<std::filesystem::path> getFilesSortedByName(std::string folder, bool includeFolders = false)
	{
		std::set<std::filesystem::path> sortedSet;

		for (const auto& it : std::filesystem::directory_iterator(folder))
		{
			if (it.is_directory() && !includeFolders)
				continue;
			sortedSet.insert(it);
		}

		return sortedSet;
	}

	inline std::ifstream openFileAtLine(const std::string& filename, uint32_t lineNumber)
	{
		std::ifstream file(filename);

		if (!file.is_open())
			return file;

		std::string line;
		for (uint32_t i = 1; i < lineNumber; i++)
		{
			if (!std::getline(file, line))
			{
				file.close();
				return std::ifstream();
			}
		}

		return file;
	}
}