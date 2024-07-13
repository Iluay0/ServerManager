#pragma once

#include "../InputModule.h"

class ProcessModule : public InputModule
{
public:
	ProcessModule();
	~ProcessModule();

	void onExit() override;

	static HWND getOpenProcess(DWORD pid);
protected:
	void bindCommands() override;
private:
	void start(std::deque<std::string> args);
	void stop(std::deque<std::string> args);
	void copy(std::deque<std::string> args);
private:
	bool m_isOpen = false;
	std::deque<std::pair<DWORD, std::string>> m_processes;
};