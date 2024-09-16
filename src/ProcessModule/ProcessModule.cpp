#include "ProcessModule.h"

#include <iostream>
#include <Windows.h>
#include <map>
#include <process.h>
#include <Tlhelp32.h>
#include <winbase.h>
#include <chrono>
#include <thread>
#include <signal.h>
#include <Psapi.h>
#include <codecvt>
#include "../LogPooler/LogPooler.h"

HWND g_hWnd = nullptr;

ProcessModule::ProcessModule()
{
    setName("process");
    setDescription("Manages the behaviour of server processes.");
    setSkipAlias(true);
    bindCommands();
}

ProcessModule::~ProcessModule()
{

}

void ProcessModule::onExit()
{
    stop({});
}

void ProcessModule::bindCommands()
{
    addCommand("start", [&](std::deque<std::string> args) { start(args); }, "Starts all server processes.");
    addCommand("restart", [&](std::deque<std::string> args) { stop(args); start(args); }, "Restarts all server processes.");
    addCommand("stop", [&](std::deque<std::string> args) { stop(args); }, "Closes all server processes.");
    addCommand("copy", [&](std::deque<std::string> args) { copy(args); }, "Copies server processes to target location(s).");
}

void ProcessModule::start(std::deque<std::string> args)
{
    if (m_isOpen)
    {
        std::cout << dye::light_red("Servers are already open.") << std::endl;
        return;
    }

    m_isOpen = true;
    m_processes.clear();

    for (const auto& [path, exeName] : util::Config::inst().getExePaths())
    {
        std::cout << "Starting " << exeName << "...";

        STARTUPINFOA si;
        PROCESS_INFORMATION pi;
        ZeroMemory(&si, sizeof(si));
        ZeroMemory(&pi, sizeof(pi));
        si.cb = sizeof(si);

        auto startCheckMethod = util::Config::inst().getStartCheckMethod();
        std::string fullPath = std::format("{}\\{}", path, exeName);
        std::string bootFileName = std::format("{}.boot", fullPath);

        if (startCheckMethod == util::StartCheckMethod::File)
        {
            std::ofstream ofs(bootFileName);
            ofs.close();
        }

        if (!CreateProcessA(fullPath.c_str(), NULL, NULL, NULL, TRUE, CREATE_NEW_CONSOLE, NULL, path.c_str(), &si, &pi))
        {
            std::cout << "[" << dye::light_red("KO") << "]" << std::endl;
            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);
            break;
        }

        std::cout << std::endl;
        m_processes.push_front({ pi.dwProcessId, exeName });
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);

        HWND hWnd = nullptr;
        while (true)
        {
            hWnd = getOpenProcess(pi.dwProcessId);
            if (hWnd != nullptr)
            {
                if (startCheckMethod == util::StartCheckMethod::Window)
                    break;

                if (startCheckMethod == util::StartCheckMethod::File)
                {
                    if (!std::filesystem::exists(bootFileName))
                        break;
                }
            }         

            if (isInterrupt())
                return;

            std::this_thread::sleep_for(10ms);
            LogPooler::inst().process();
        }

        PostMessage(hWnd, WM_SYSCOMMAND, SC_MINIMIZE, 0);

        std::cout << "[" << dye::light_green("OK") << "]" << std::endl;
        std::this_thread::sleep_for(10ms);
    }
}

void ProcessModule::stop(std::deque<std::string> args)
{
    if (!m_isOpen)
    {
        std::cout << "Servers are already closed." << std::endl;
        return;
    }

    for (const auto& [pid, exeName] : m_processes)
    {
        std::cout << "Closing " << exeName << "...";

        HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, pid);
        if (hProcess == nullptr)
        {
            std::cout << dye::light_red("Failed to find process. Error: ") << dye::light_red(GetLastError()) << std::endl;
            continue;
        }

        BOOL result = TerminateProcess(hProcess, 0);
        if (!result)
        {
            std::cout << dye::light_red("Failed to terminate process. Error: ") << dye::light_red(GetLastError()) << std::endl;
            CloseHandle(hProcess);
            continue;
        }

        CloseHandle(hProcess);

        std::cout << "[" << dye::light_green("OK") << "]" << std::endl;
        std::this_thread::sleep_for(100ms);

        if (isInterrupt())
            break;
    }
    m_processes.clear();

    m_isOpen = false;
}

void ProcessModule::copy(std::deque<std::string> args)
{
    if (m_isOpen)
    {
        std::cout << "Servers are currently open." << std::endl;
        return;
    }

    for (const auto& [from, to] : util::Config::inst().getCopyPaths())
    {
        try
        {
            std::cout << "Copying " + from + " to " + to + "..." << std::endl;
            std::filesystem::copy(from, to, std::filesystem::copy_options::overwrite_existing);
            std::cout << "[" << dye::light_green("OK") << "]" << std::endl;
        }
        catch (const std::filesystem::filesystem_error& ex)
        {
            std::cout << "what():  " << ex.what() << '\n'
                << "path1(): " << ex.path1() << '\n'
                << "path2(): " << ex.path2() << '\n'
                << "code().value():    " << ex.code().value() << '\n'
                << "code().message():  " << ex.code().message() << '\n'
                << "code().category(): " << ex.code().category().name() << '\n';
        }
    }
}

HWND ProcessModule::getOpenProcess(DWORD pid)
{
    g_hWnd = nullptr;

    EnumWindows([](HWND hwnd, LPARAM lParam) -> BOOL {
        auto& pid = *reinterpret_cast<DWORD*>(lParam);
        DWORD lpdwProcessId;
        GetWindowThreadProcessId(hwnd, &lpdwProcessId);
        if (lpdwProcessId == pid)
        {
            g_hWnd = hwnd;
            return FALSE;
        }
        return TRUE;
    }, reinterpret_cast<LPARAM>(&pid));

    if (g_hWnd == nullptr || !IsWindow(g_hWnd) || !IsWindowVisible(g_hWnd))
        return nullptr;

    return g_hWnd;
}