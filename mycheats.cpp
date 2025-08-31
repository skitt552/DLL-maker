// cheats.cpp
#include <windows.h>
#include <iostream>

// ==========================
// Configurable Settings
// ==========================
struct Config {
    bool aimbot = true;
    bool espBox = true;
    bool espSkeleton = true;
    bool radar = true;
    bool triggerbot = true;
} config;

bool running = true;

// ==========================
// Print Status Menu
// ==========================
void PrintMenu() {
    system("cls"); // clear console for clean menu
    std::cout << "==============================\n";
    std::cout << "        Cheat Menu (F-Keys)   \n";
    std::cout << "==============================\n";
    std::cout << "[F1] Aimbot       : " << (config.aimbot ? "ON" : "OFF") << "\n";
    std::cout << "[F2] ESP Box      : " << (config.espBox ? "ON" : "OFF") << "\n";
    std::cout << "[F3] Skeleton ESP : " << (config.espSkeleton ? "ON" : "OFF") << "\n";
    std::cout << "[F4] Radar        : " << (config.radar ? "ON" : "OFF") << "\n";
    std::cout << "[F5] Triggerbot   : " << (config.triggerbot ? "ON" : "OFF") << "\n";
    std::cout << "------------------------------\n";
    std::cout << "[F6] Unload DLL & Exit\n";
    std::cout << "==============================\n";
}

// ==========================
// Main Hack Thread
// ==========================
DWORD WINAPI HackThread(HMODULE hModule) {
    // Open console
    AllocConsole();
    FILE* f;
    freopen_s(&f, "CONOUT$", "w", stdout);

    PrintMenu(); // show menu first time

    while (running) {
        if (GetAsyncKeyState(VK_F1) & 1) { config.aimbot = !config.aimbot; PrintMenu(); }
        if (GetAsyncKeyState(VK_F2) & 1) { config.espBox = !config.espBox; PrintMenu(); }
        if (GetAsyncKeyState(VK_F3) & 1) { config.espSkeleton = !config.espSkeleton; PrintMenu(); }
        if (GetAsyncKeyState(VK_F4) & 1) { config.radar = !config.radar; PrintMenu(); }
        if (GetAsyncKeyState(VK_F5) & 1) { config.triggerbot = !config.triggerbot; PrintMenu(); }

        if (GetAsyncKeyState(VK_F6) & 1) {
            std::cout << "\n[+] Unloading DLL...\n";
            running = false;
        }

        Sleep(100); // avoid CPU overuse
    }

    fclose(f);
    FreeConsole();
    FreeLibraryAndExitThread(hModule, 0);
    return 0;
}

// ==========================
// DLL Entry Point
// ==========================
BOOL WINAPI DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved) {
    if (dwReason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hModule);
        CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)HackThread, hModule, 0, nullptr);
    }
    return TRUE;
}