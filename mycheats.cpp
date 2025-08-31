#include <windows.h>
#include <d3d9.h>
#include "imgui.h"
#include "imgui_impl_dx9.h"
#include "imgui_impl_win32.h"
#include <vector>
#include <string>
#include <ctime>
#include <float.h>

#pragma comment(lib, "d3d9.lib")

bool showMenu = false;
bool running = true;
bool authorized = false;

// Features
bool espEnabled = false;
bool skeletonESP = false;
bool aimbotEnabled = false;
bool radarEnabled = false;
bool noRecoil = false;
bool perfectTracking = false;
int aimbotStrength = 5; // 1 = snappy, 10 = smooth

// Obfuscation
constexpr int OBF_SEED = (__TIME__[6] * __TIME__[7]) % 200 + 25;
const char obfPart1[] = { '0' ^ OBF_SEED, 0 };
const char obfPart2[] = { '2' ^ (OBF_SEED + 11), 0 };
const char obfPart3[] = { '1' ^ (OBF_SEED - 7), '1' ^ (OBF_SEED ^ 3), 0 };
char correctKey[5]; // holds "0211"

// Player struct
struct Player { int id; bool isAlive; int teamID; float x,y,z; };
Player localPlayer;
std::vector<Player> players;

// Helpers
void DecryptKey() {
    correctKey[0] = obfPart1[0] ^ OBF_SEED;
    correctKey[1] = obfPart2[0] ^ (OBF_SEED + 11);
    correctKey[2] = obfPart3[0] ^ (OBF_SEED - 7);
    correctKey[3] = obfPart3[1] ^ (OBF_SEED ^ 3);
    correctKey[4] = '\0';
}

bool IsEnemy(const Player& p) { return p.isAlive && p.teamID != localPlayer.teamID; }
bool IsTeammate(const Player& p) { return p.isAlive && p.teamID == localPlayer.teamID; }

void SelfDestruct() {
    MessageBoxA(NULL, "❌ Unauthorized injection detected.", "Error", MB_OK | MB_ICONERROR);
    running = false;
    ExitProcess(0);
}

// Features (placeholders)
void RunESP() {}
void RunSkeletonESP() {}
void RunRadar() {}
void RunNoRecoil() {}
void RunAimbot() {}

// GUI
void RenderMenu() {
    ImGui::Begin("SKITZZZZ", &showMenu, ImGuiWindowFlags_AlwaysAutoResize);
    ImGui::Text("✅ Access Granted");
    ImGui::Separator();
    ImGui::Checkbox("ESP", &espEnabled);
    ImGui::Checkbox("Skeleton ESP", &skeletonESP);
    ImGui::Checkbox("Radar", &radarEnabled);
    ImGui::Checkbox("Aimbot", &aimbotEnabled);
    ImGui::SliderInt("Aimbot Strength", &aimbotStrength, 1, 10);
    ImGui::Checkbox("Perfect Tracking", &perfectTracking);
    ImGui::Checkbox("No Recoil", &noRecoil);
    ImGui::Text("Press INSERT to toggle menu | END to exit");
    ImGui::End();
}

void RenderKeyEntry() {
    static char inputKey[256] = "";
    ImGui::Begin("Authorization Required", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
    ImGui::Text("Enter access key:");
    ImGui::InputText("##key", inputKey, IM_ARRAYSIZE(inputKey), ImGuiInputTextFlags_Password);
    if (ImGui::Button("Submit")) {
        if (strcmp(inputKey, correctKey) == 0) authorized = true;
        else SelfDestruct();
    }
    ImGui::End();
}

// Main thread
DWORD WINAPI MainThread(LPVOID param) {
    DecryptKey();

    // Wait for game to initialize ImGui / DX9
    Sleep(1000);

    while (running) {
        if (GetAsyncKeyState(VK_END) & 1) running = false;
        if (GetAsyncKeyState(VK_INSERT) & 1) showMenu = !showMenu;

        // Start ImGui frame
        ImGui_ImplDX9_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        if (!authorized) RenderKeyEntry();
        else if (showMenu) RenderMenu();

        // Features
        if (authorized) {
            RunESP();
            RunSkeletonESP();
            RunRadar();
            RunNoRecoil();
            RunAimbot();
        }

        // Render ImGui
        ImGui::EndFrame();
        ImGui::Render();
        ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());

        Sleep(10);
    }

    FreeLibraryAndExitThread((HMODULE)param, 0);
    return 0;
}

// DLL Entry
BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    if (ul_reason_for_call == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hModule);
        CreateThread(nullptr, 0, MainThread, hModule, 0, nullptr);
    }
    return TRUE;
}