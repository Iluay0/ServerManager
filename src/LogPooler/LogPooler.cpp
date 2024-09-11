#include "LogPooler.h"

LogPooler::LogPooler()
{

}

LogPooler::~LogPooler()
{

}

void LogPooler::setup()
{
	m_startTime = std::time(nullptr);
}

void LogPooler::process()
{
	const auto& config = util::Config::inst().getLogPoolerConfig();
	if (!config.m_enabled)
		return;

	time_t t = std::time(nullptr);
	tm* now = std::localtime(&t);
	int year = now->tm_year + 1900;
	int month = now->tm_mon + 1;
	int day = now->tm_mday;

	std::string regexFileName = config.m_regexFileName;
	regexFileName = std::regex_replace(regexFileName, std::regex("Y-m-d"), std::format("{:04d}-{:02d}-{:02d}", year, month, day));

	for (const auto& folder : config.m_folders)
	{
		for (const auto& it : std::filesystem::recursive_directory_iterator(folder))
		{
			if (it.is_directory())
				continue;

			auto system_time = std::chrono::clock_cast<std::chrono::system_clock>(it.last_write_time());
			std::time_t lastEditTime = std::chrono::system_clock::to_time_t(system_time);

			std::smatch matchFileName;
			std::string filename = it.path().filename().string();
			if (!std::regex_match(filename, matchFileName, std::regex(regexFileName)))
				continue;

			std::smatch matchExeName;
			if (!std::regex_search(filename, matchExeName, std::regex(config.m_regexExeName)))
				continue;

			if (!m_mapLastScannedLine.count(filename))
				m_mapLastScannedLine[filename] = 0;

			std::ifstream file = util::openFileAtLine(it.path().string(), m_mapLastScannedLine[filename]);
			if (!file.is_open())
				continue;

			std::string line;
			while (std::getline(file, line))
			{
				if (lastEditTime >= m_startTime)
				{
					std::cout << "[" << dye::light_aqua(matchExeName[0]) << "]";
					printLog(line);
				}
				m_mapLastScannedLine[filename]++;
			}
			file.close();
		}
	}
}

void LogPooler::printLog(std::string log)
{
	if (log.find("(Critical)") != std::string::npos)
		std::cout << dye::red(log);
	else if (log.find("(Error)") != std::string::npos)
		std::cout << dye::light_red(log);
	else if (log.find("(Warning)") != std::string::npos)
		std::cout << dye::yellow(log);
	else
		std::cout << dye::grey(log);

	std::cout << std::endl;
}
