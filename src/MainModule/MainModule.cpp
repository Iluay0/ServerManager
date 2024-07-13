#include "MainModule.h"

#include <iostream>
#include "../ProcessModule/ProcessModule.h"
#include "../DbModule/DbModule.h"

MainModule::MainModule()
{
    setName("main");
    setDescription("Manages the global commands of this application.");
    setSkipAlias(true);
    bindCommands();
    createChildModules();
}

MainModule::~MainModule()
{
}

void MainModule::bindCommands()
{
    addCommand("exit", [&](std::deque<std::string> args) { m_running = false; }, "Exits the program, closing all executables.");
    addCommand("help", [&](std::deque<std::string> args) { help(args); }, "Prints the help message.");
}

void MainModule::createChildModules()
{
    addModule(std::shared_ptr<ProcessModule>(new ProcessModule()));
    addModule(std::shared_ptr<DbModule>(new DbModule()));
}

void MainModule::help(std::deque<std::string> args)
{
    switch (args.size())
    {
        case 0:
        {
            print();
            std::cout << "Enter 'help <module name>' for further information." << std::endl;
        } break;
        case 1:
        {
            auto split = util::splitString<std::deque<std::string>>(args.back(), ":");

            auto module = getModule(split.front());
            for (const auto& it: split | std::views::drop(1))
            {
                if (module)
                    module = module->getModule(it);
            }

            if (!module)
            {
                std::cout << dye::light_red("Invalid module.") << std::endl;
                return;
            }

            module->print();

            std::cout << "Enter 'help <module name>' for further information." << std::endl;
        } break;
        default:
        {
            std::cout << dye::light_red("Too many arguments.") << std::endl;
        } break;
    }
}
