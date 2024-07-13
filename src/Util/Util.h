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

#include <color.hpp>
#include <json.hpp>

using namespace std::chrono_literals;

constexpr auto configFile = ".\\config.json";
constexpr auto dbLastMigrationFile = ".\\latest_migration.txt";
constexpr auto dbMigrationsFolder = ".\\DBMigrations";
constexpr auto migrationDateFormat = "%Y-%m-%d-%H%M%S";

namespace util
{
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
			}
			catch (nlohmann::json::exception e)
			{
				std::cout << dye::light_red("Could not parse file: " + std::string(configFile) + ".") << std::endl;
			}
		}

		std::string getDbInstanceName() { return m_dbInstanceName; }
		const std::deque<std::pair<std::string, std::string>>& getExePaths() { return m_exePaths; }
		const std::deque<std::pair<std::string, std::string>>& getCopyPaths() { return m_copyPaths; }

		static Config& inst()
		{
			static Config instance;
			return instance;
		}
	private:
		std::string m_dbInstanceName = "";
		std::deque<std::pair<std::string, std::string>> m_exePaths;
		std::deque<std::pair<std::string, std::string>> m_copyPaths;
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
}