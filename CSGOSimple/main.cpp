#define NOMINMAX
#include <Windows.h>

#include "valve_sdk/sdk.hpp"
#include "helpers/utils.hpp"
#include "helpers/input.hpp"
#include "features/skins/parser.hpp"
#include "hooks.hpp"
#include "menu.hpp"
#include "options.hpp"
#include "render.hpp"
#include <thread>
#include "config.hpp"

DWORD WINAPI OnDllAttach(LPVOID base)
{
    while (!GetModuleHandleA("client.dll"))
        Sleep(1000);
    while (!GetModuleHandleA("engine.dll"))
        Sleep(1000);
	while (!GetModuleHandleA("serverbrowser.dll"))
		Sleep(1000);
    while (!GetModuleHandleA("studiorender.dll"))
        Sleep(1000);
    while (!GetModuleHandleA("materialsystem.dll"))
        Sleep(1000);

#ifdef _DEBUG
    Utils::AttachConsole();
#endif

    try {
        Utils::ConsolePrint("Initializing...\n");

        Interfaces::Initialize();
        Interfaces::Dump();

        NetvarSys::Get().Initialize();
        InputSys::Get().Initialize();
		Render::Get().Initialize();
        Menu::Get().Initialize();

        Config->Setup();

        game_data::initialize_kits();

        Hooks::Initialize();

        // Register some hotkeys.
        // - Note:  The function that is called when the hotkey is pressed
        //          is called from the WndProc thread, not this thread.
        // 

        // Panic button
        //InputSys::Get().RegisterHotkey(VK_DELETE, [base]() {
        //    g_Unload = true;
        //});

        // Menu Toggle
        InputSys::Get().RegisterHotkey(VK_INSERT, [base]() {
            Menu::Get().Toggle();
        });

        Utils::ConsolePrint("Bypassing anticheat...\n");

        LPCWSTR modules[]{ L"client.dll", L"engine.dll", L"server.dll", L"studiorender.dll", L"materialsystem.dll" };
        long long patch = 0x69690004C201B0;
        for (auto base : modules)
            WriteProcessMemory(GetCurrentProcess(), Utils::PatternScan(GetModuleHandleW(base), "55 8B EC 56 8B F1 33 C0 57 8B 7D 08"), &patch, 5, 0);

        Utils::ConsolePrint("Finished.\n");
		Utils::ConsolePrint("Built on: %s %s\n", __DATE__, __TIME__);

        while (!g_Unload)
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));

        g_CVar->FindVar("crosshair")->SetValue(true);

        FreeLibraryAndExitThread(static_cast<HMODULE>(base), 1);

    } catch(const std::exception& ex) {
        Utils::ConsolePrint("An error occured during initialization:\n");
        Utils::ConsolePrint("%s\n", ex.what());
        Utils::ConsolePrint("Press any key to exit.\n");
        Utils::ConsoleReadKey();
        Utils::DetachConsole();

        FreeLibraryAndExitThread(static_cast<HMODULE>(base), 1);
    }

    // unreachable
    //return TRUE;
}

BOOL WINAPI OnDllDetach()
{
#ifdef _DEBUG
    Utils::DetachConsole();
#endif

    Hooks::Shutdown();

    Menu::Get().Shutdown();
    return TRUE;
}

BOOL WINAPI DllMain(
    _In_      HINSTANCE hinstDll,
    _In_      DWORD     fdwReason,
    _In_opt_  LPVOID    lpvReserved
)
{
    switch(fdwReason) {
        case DLL_PROCESS_ATTACH:
            DisableThreadLibraryCalls(hinstDll);
            CreateThread(nullptr, 0, OnDllAttach, hinstDll, 0, nullptr);
            return TRUE;
        case DLL_PROCESS_DETACH:
            if(lpvReserved == nullptr)
                return OnDllDetach();
            return TRUE;
        default:
            return TRUE;
    }
}
