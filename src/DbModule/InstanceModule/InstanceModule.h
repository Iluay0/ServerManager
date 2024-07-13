#pragma once

#include "../../InputModule.h"

class InstanceModule : public InputModule
{
public:
	InstanceModule();
	~InstanceModule();

	static std::string getInstanceName();
protected:
	void bindCommands() override;
private:
	void setInstanceName(std::deque<std::string> args);
};