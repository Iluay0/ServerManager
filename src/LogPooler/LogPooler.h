#pragma once

#include "../Util/Util.h"

class LogPooler
{
	std::map<std::string, uint32_t> m_mapLastScannedLine;
	time_t m_startTime;
public:
	LogPooler();
	~LogPooler();

	void setup();
	void process();

	void printLog(std::string log);

	static LogPooler& inst()
	{
		static LogPooler instance;
		return instance;
	}
};