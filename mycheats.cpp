#include <windows.h>
#include <d3d9.h>
#include "imgui.h"
#include "imgui_impl_dx9.h"
#include "imgui_impl_win32.h"
#include <ctime>
#include <string>

#pragma comment(lib, "d3d9.lib")

// Globals
bool showMenu = false;
bool running = true;
bool authorized = false;

// Build-time obfuscation seed (changes every build)
constexpr int OBF_SEED = (__TIME__[6] * __TIME__[7]) % 200 + 25;

// Multi-layer key storage ("0211"), broken into chunks
const char obfPart1[] = { '0' ^ OBF_SEED, 0 };
const char obfPart2[] = { '2' ^ (OBF_SEED + 11), 0 };
const char obfPart3[] = { '1' ^ (OBF_SEED - 7), '1' ^ (OBF_SEED ^ 3), 0 };

char correctKey[5]; // will hold "0211"

// Forward declarations
DWORD WINAPI MainThread(LPVOID param);
void SelfDestruct();
void RenderMenu();
void RenderKeyEntry();
void DecryptKey();

// Runtime decryption
void DecryptKey()
{
    // Rebuild dynamically from parts
    correctKey[0] = obfPart1[0] ^ OBF_SEED;
    correctKey[1] = obfPart2[0] ^ (OBF_SEED + 11);
    correctKey[2] = obfPart3[0] ^ (OBF_SEED - 7);
    correctKey[3] = obfPart3[1] ^ (OBF_SEED ^ 3);
    correctKey[4] = '\0';
}

// Self destruct (for wrong key/forced injection)
void SelfDestruct()
{
    MessageBoxA(NULL, "❌ Unauthorized injection detected. Self-destructing...", "Error", MB_OK | MB_ICONERROR);
    running = false;

    char path[MAX_PATH];
    GetModuleFileNameA((HMODULE)GetModuleHandle(NULL), path, MAX_PATH);
    DeleteFileA(path);

    ExitProcess(0);
}

// Menu GUI
void RenderMenu()
{
    ImGui::Begin("SKITZZZZ", &showMenu, ImGuiWindowFlags_AlwaysAutoResize);
    ImGui::Text("✅ Access Granted");
    ImGui::Separator();
    ImGui::Text("Press INSERT to toggle this menu");
    ImGui::Text("Press END to exit cleanly");
    ImGui::End();
}

// Key entry GUI
void RenderKeyEntry()
{
    static char inputKey[256] = "";
    ImGui::Begin("Authorization Required", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
    ImGui::Text("Enter access key:");
    ImGui::InputText("##key", inputKey, IM_ARRAYSIZE(inputKey), ImGuiInputTextFlags_Password);

    if (ImGui::Button("Submit")) {
        if (strcmp(inputKey, correctKey) == 0) {
            authorized = true;
            MessageBoxA(NULL, "✅ Access Granted!", "SKITZZZZ", MB_OK | MB_ICONINFORMATION);
        } else {
            SelfDestruct();
        }
    }
    ImGui::End();
}

// Main Thread
DWORD WINAPI MainThread(LPVOID param)
{
    DecryptKey();

    while (running) {
        if (GetAsyncKeyState(VK_END) & 1) {
            running = false;
        }

        if (!authorized) {
            RenderKeyEntry();
        } else {
            if (GetAsyncKeyState(VK_INSERT) & 1) {
                showMenu = !showMenu;
            }
            if (showMenu) {
                RenderMenu();
            }
        }
        Sleep(10);
    }

    FreeLibraryAndExitThread((HMODULE)param, 0);
    return 0;
}

// DLL entry
BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    if (ul_reason_for_call == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hModule);
        CreateThread(nullptr, 0, MainThread, hModule, 0, nullptr);
    }
    return TRUE;
}