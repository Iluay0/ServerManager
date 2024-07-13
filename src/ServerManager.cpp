#include "Util/Util.h"

#include "InputModule.h"
#include "MainModule/MainModule.h"

#include "DbModule/MigrationModule/MigrationModule.h"

using namespace std::chrono_literals;

std::shared_ptr<MainModule> m_MainModule;

BOOL WINAPI ctrlHandler(DWORD fdwCtrlType)
{
    switch (fdwCtrlType)
    {
        // Handle the CTRL-C signal.
        case CTRL_C_EVENT:
            std::cout << "[Ctrl-C]" << std::endl;
            for (auto& [name, module] : m_MainModule->getModules())
            {
                module->setInterrupt(true);
            }
            return TRUE;
        default:
            return FALSE;
    }
}

bool onInput(std::string input)
{
    if (m_MainModule->onInput(input))
        return true;

    for (auto& [name, module] : m_MainModule->getModules())
    {
        if (module->onInput(input))
            return true;
    }

    return false;
}

void setup()
{
    SetConsoleCtrlHandler(ctrlHandler, TRUE);

    m_MainModule = std::shared_ptr<MainModule>(new MainModule());

    util::Config::inst().load();

    std::cout << dye::light_aqua("Welcome to ServerManager!") << std::endl;
    std::cout << dye::light_aqua("Enter 'help' for a list of commands.") << std::endl;

    int pendingMigrations = MigrationModule::getPendingMigrations();
    if (pendingMigrations > 0)
        std::cout << dye::on_light_red("You have " + std::to_string(pendingMigrations) + " migrations pending. Run ") << dye::on_red("db:migration:run") << dye::on_light_red(" to update.");
}

int main()
{
    setup();

    while (m_MainModule->isRunning())
    {
        std::cout << std::endl << "Waiting for input..." << std::endl;

        std::string szInput;
        getline(std::cin, szInput);
        std::cout << std::endl;

        for (auto& [name, module] : m_MainModule->getModules())
        {
            module->setInterrupt(false);
        }

        if (!onInput(szInput))
            std::cout << dye::light_red("Unrecognized command.") << std::endl;

        for (auto& [name, module] : m_MainModule->getModules())
        {
            module->setInterrupt(false);
        }
    }

    for (auto& [name, module] : m_MainModule->getModules())
    {
        module->onExit();
    }
}