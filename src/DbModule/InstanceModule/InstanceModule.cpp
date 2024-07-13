#include "InstanceModule.h"

InstanceModule::InstanceModule()
{
    setName("instance");
    setDescription("Manages database instance data.");
    setSkipAlias(false);
    bindCommands();
}

InstanceModule::~InstanceModule()
{

}

std::string InstanceModule::getInstanceName()
{
    return util::Config::inst().getDbInstanceName();
}

void InstanceModule::bindCommands()
{
    addCommand("set_name", [&](std::deque<std::string> args) { setInstanceName(args); }, "Sets the name of the database instance. db:instance:set_name <ComputerName>\\<InstanceName>.");
}

void InstanceModule::setInstanceName(std::deque<std::string> args)
{
    if (args.size() != 1)
    {
        std::cout << dye::light_red("Too many or too few arguments. Accepted argument(s): '<ComputerName>\\<InstanceName>'.") << std::endl;
        return;
    }

    std::ifstream inputFile(configFile);
    nlohmann::json json = nlohmann::json::parse(inputFile);
    json["dbInstanceName"] = args.back();
    inputFile.close();

    std::ofstream outputFile(configFile);
    outputFile << std::setw(4) << json;
    outputFile.close();

    std::cout << dye::light_green("Database instance name has been set.") << std::endl;
}