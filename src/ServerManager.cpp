#include "Util/Util.h"

#include "InputModule.h"
#include "MainModule/MainModule.h"

#include "DbModule/MigrationModule/MigrationModule.h"
#include "LogPooler/LogPooler.h"

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
    }
    return TRUE;
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
    LogPooler::inst().setup();

    std::cout << dye::light_aqua("Welcome to ServerManager!") << std::endl;
    std::cout << dye::light_aqua("Enter 'help' for a list of commands.") << std::endl;

    int pendingMigrations = MigrationModule::getPendingMigrations();
    if (pendingMigrations > 0)
        std::cout << dye::on_light_red("You have " + std::to_string(pendingMigrations) + " migrations pending. Run ") << dye::on_red("db:migration:run") << dye::on_light_red(" to update.") << std::endl;

    std::cout << std::endl << "Waiting for input..." << std::endl;
}

int main()
{
    using namespace std::literals;

    setup();

    while (m_MainModule->isRunning())
    {
        auto f = std::async(std::launch::async, [] {
            auto s = ""s;
            if (std::cin >> s) return s;
        });

        while (f.wait_for(100ms) != std::future_status::ready)
        {
            LogPooler::inst().process();
        }

        for (auto& [name, module] : m_MainModule->getModules())
        {
            module->setInterrupt(false);
        }

        std::string szInput = f.get();
        if (szInput != "" && !onInput(szInput))
            std::cout << dye::light_red("Unrecognized command.") << std::endl;

        for (auto& [name, module] : m_MainModule->getModules())
        {
            module->setInterrupt(false);
        }

        if (std::cin.fail())
        {
            std::cin.clear();
        }

        std::cout << std::endl << "Waiting for input..." << std::endl;
    }

    for (auto& [name, module] : m_MainModule->getModules())
    {
        module->onExit();
    }
}