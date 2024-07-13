#include "DbModule.h"

#include "MigrationModule/MigrationModule.h"
#include "InstanceModule/InstanceModule.h"

DbModule::DbModule()
{
    setName("db");
    setDescription("Manages database-related submodules.");
    setSkipAlias(false);
    bindCommands();
    createChildModules();
}

DbModule::~DbModule()
{
}

void DbModule::createChildModules()
{
    addModule(std::shared_ptr<InstanceModule>(new InstanceModule()));
    addModule(std::shared_ptr<MigrationModule>(new MigrationModule()));
}
