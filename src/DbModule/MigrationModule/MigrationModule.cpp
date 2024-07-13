#include "MigrationModule.h"

#include "../InstanceModule/InstanceModule.h"

MigrationModule::MigrationModule()
{
    setName("migration");
    setDescription("Manages database migrations.");
    setSkipAlias(false);
    bindCommands();
}

MigrationModule::~MigrationModule()
{

}

std::string MigrationModule::getLastMigration()
{
    std::ifstream file(dbLastMigrationFile);
    if (file.is_open())
    {
        std::string line;
        if (getline(file, line))
            return line;
    }
    else
    {
        std::filesystem::path path = dbLastMigrationFile;
        std::filesystem::create_directories(path.parent_path());

        std::ofstream fileInput(path);
        if (fileInput.is_open())
        {
            fileInput << util::getTimeString(0, migrationDateFormat);
            fileInput.close();
            return getLastMigration();
        }
        std::cout << dye::light_red("Could not create file: " + path.string() + ".") << std::endl;
    }
    return util::getTimeString(0, migrationDateFormat);
}

std::string MigrationModule::getMigrationDate(std::string szFilePath)
{
    auto split = util::splitString<std::deque<std::string>>(szFilePath, "_");
    return split.front();
}

int MigrationModule::getPendingMigrations()
{
    std::filesystem::create_directories(dbMigrationsFolder);

    int amount = 0;
    std::string lastMigration = MigrationModule::getLastMigration();
    time_t secLastMigration = util::getUnixTime(lastMigration, migrationDateFormat);

    for (const auto& it : util::getFilesSortedByName(dbMigrationsFolder))
    {
        time_t secMigration = util::getUnixTime(MigrationModule::getMigrationDate(it.filename().string()), migrationDateFormat);
        if (secMigration <= secLastMigration)
            continue;
        amount++;
    }

    return amount;
}

void MigrationModule::bindCommands()
{
    addCommand("make", [&](std::deque<std::string> args) { make(args); }, "Creates a migration file. db:migration:make migration_name");
    addCommand("run", [&](std::deque<std::string> args) { run(args); }, "Runs all database migrations.");
    addCommand("list", [&](std::deque<std::string> args) { list(args); }, "Lists all database migrations and their status.");
}

void MigrationModule::make(std::deque<std::string> args)
{
    if (args.size() != 1)
    {
        std::cout << dye::light_red("Too many or too few arguments. Format: 'db:migration:make migration_name'.") << std::endl;
        return;
    }

    std::filesystem::path path{ dbMigrationsFolder };
    std::string fileName = util::getTimeString(std::time(0), migrationDateFormat) + "_" + args.back() + ".sql";
    std::filesystem::create_directories(path.parent_path());
    path /= fileName;

    std::ofstream file(path);
    if (file.is_open())
    {
        file << "-- Database migration: " << fileName;
        file.close();
        std::cout << dye::light_green("Migration has been created at the following path: " + path.string()) << std::endl;
    }
    else
    {
        std::cout << dye::light_red("Could not open file: " + path.string() + ".") << std::endl;
    }
}

void MigrationModule::run(std::deque<std::string> args)
{
    std::string instanceName = InstanceModule::getInstanceName();
    if (instanceName == "")
    {
        std::cout << dye::light_red("No instance name has been set. Use 'db:instance:set_name' to set an instance name.") << std::endl;
        return;
    }

    std::string lastMigration = getLastMigration();
    std::cout << "Last migration: " << dye::on_light_aqua(lastMigration) << std::endl;
    time_t secLastMigration = util::getUnixTime(lastMigration, migrationDateFormat);

    std::cout << "Running migrations..." << std::endl;

    for (const auto& it : util::getFilesSortedByName(dbMigrationsFolder))
    {
        time_t secMigration = util::getUnixTime(getMigrationDate(it.filename().string()), migrationDateFormat);
        if (secMigration <= secLastMigration)
            continue;

        std::cout << "Running " << it.string() << "...";
        std::string command = "sqlcmd -S " + instanceName + " -E -b -i \"" + std::filesystem::absolute(it).string() + "\"";
        if (std::system(command.c_str()) != 0)
        {
            std::cout << "[" << dye::light_red("KO") << "]" << std::endl;
            std::cout << "Error: " << strerror(errno) << std::endl;
            break;
        }
        std::cout << "[" << dye::light_green("OK") << "]" << std::endl;
        setLastMigration(getMigrationDate(it.filename().string()));
        std::this_thread::sleep_for(200ms);

        if (isInterrupt())
            break;
    }
    std::cout << dye::light_green("Up to date!") << std::endl;
}

void MigrationModule::list(std::deque<std::string> args)
{
    std::string lastMigration = getLastMigration();
    std::cout << "Last migration: " << dye::on_light_aqua(lastMigration) << std::endl;
    time_t secLastMigration = util::getUnixTime(lastMigration, migrationDateFormat);

    std::cout << "Migration list: " << std::endl;
    for (const auto& it : util::getFilesSortedByName(dbMigrationsFolder))
    {
        time_t secMigration = util::getUnixTime(getMigrationDate(it.filename().string()), migrationDateFormat);

        auto status = dye::light_green("OK");
        if (secMigration > secLastMigration)
            status = dye::light_red("KO");

        std::cout << "[" << status << "]" << " " << it.filename().string() << std::endl;

        if (isInterrupt())
            return;
    }
}

void MigrationModule::setLastMigration(std::string migrationDate)
{
    std::filesystem::path path{ dbLastMigrationFile };
    std::filesystem::create_directories(path.parent_path());

    std::ofstream file{ dbLastMigrationFile };
    if (file.is_open())
    {
        file << migrationDate;
        file.close();
    }
    else
    {
        std::cout << dye::light_red("Could not open file: " + std::string(dbLastMigrationFile) + ".") << std::endl;
    }
}