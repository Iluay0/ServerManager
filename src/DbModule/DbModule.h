#pragma once

#include "../InputModule.h"

class DbModule : public InputModule
{
public:
	DbModule();
	~DbModule();
protected:
	void createChildModules() override;
};