#include <windows.h>
#include <d3d9.h>
#include "imgui.h"
#include "imgui_impl_dx9.h"
#include "imgui_impl_win32.h"
#include <vector>
#include <string>
#include <ctime>

#pragma comment(lib, "d3d9.lib")

// =====================================================
// Globals
// =====================================================
bool showMenu = false;
bool running = true;
bool authorized = false;

bool espEnabled = false;
bool skeletonESP = false;
bool aimbotEnabled = false;
bool radarEnabled = false;
bool noRecoil = false;
bool perfectTracking = false;

int aimbotStrength = 5; // 1 = snappy, 10 = smooth assist

// Build-time obfuscation seed
constexpr int OBF_SEED = (__TIME__[6] * __TIME__[7]) % 200 + 25;
const char obfPart1[] = { '0' ^ OBF_SEED, 0 };
const char obfPart2[] = { '2' ^ (OBF_SEED + 11), 0 };
const char obfPart3[] = { '1' ^ (OBF_SEED - 7), '1' ^ (OBF_SEED ^ 3), 0 };
char correctKey[5]; // holds "0211"

// =====================================================
// Player structure (replace with real game structs)
// =====================================================
struct Player {
    int id;
    bool isAlive;
    int teamID;
    float x, y, z;
};

Player localPlayer;
std::vector<Player> players;

// =====================================================
// Helpers
// =====================================================
void DecryptKey() {
    correctKey[0] = obfPart1[0] ^ OBF_SEED;
    correctKey[1] = obfPart2[0] ^ (OBF_SEED + 11);
    correctKey[2] = obfPart3[0] ^ (OBF_SEED - 7);
    correctKey[3] = obfPart3[1] ^ (OBF_SEED ^ 3);
    correctKey[4] = '\0';
}

bool IsEnemy(const Player& p) {
    return p.isAlive && p.teamID != localPlayer.teamID;
}

bool IsTeammate(const Player& p) {
    return p.isAlive && p.teamID == localPlayer.teamID;
}

void SelfDestruct() {
    MessageBoxA(NULL, "❌ Unauthorized injection detected. Self-destructing...", "Error", MB_OK | MB_ICONERROR);
    running = false;
    char path[MAX_PATH];
    GetModuleFileNameA((HMODULE)GetModuleHandle(NULL), path, MAX_PATH);
    DeleteFileA(path);
    ExitProcess(0);
}

// =====================================================
// Features (stubs — replace with game logic)
// =====================================================
void RunESP() {
    if (!espEnabled) return;
    for (auto& p : players) {
        if (!p.isAlive) continue;

        ImU32 color;
        if (IsEnemy(p)) color = IM_COL32(255, 0, 0, 255);       // Red = enemy
        else if (IsTeammate(p)) color = IM_COL32(0, 255, 0, 255); // Green = teammate
        else color = IM_COL32(200, 200, 200, 255);

        // TODO: WorldToScreen
        // Example:
        // ImGui::GetBackgroundDrawList()->AddRect(ImVec2(x, y), ImVec2(x+50, y+100), color);
    }
}

void RunSkeletonESP() {
    if (!skeletonESP) return;
    for (auto& p : players) {
        if (!p.isAlive) continue;

        ImU32 color = IsEnemy(p) ? IM_COL32(255, 0, 0, 255) : IM_COL32(0, 255, 0, 255);

        // TODO: Draw lines between bones
        // ImGui::GetBackgroundDrawList()->AddLine(start, end, color, 1.5f);
    }
}

void RunRadar() {
    if (!radarEnabled) return;

    float radarX = 50, radarY = 50, radarSize = 150;
    ImDrawList* drawList = ImGui::GetBackgroundDrawList();

    // Radar background
    drawList->AddRectFilled(ImVec2(radarX, radarY), ImVec2(radarX + radarSize, radarY + radarSize), IM_COL32(30, 30, 30, 150));

    for (auto& p : players) {
        if (!p.isAlive) continue;

        ImU32 color;
        if (IsEnemy(p)) color = IM_COL32(255, 0, 0, 255);
        else if (IsTeammate(p)) color = IM_COL32(0, 255, 0, 255);
        else if (p.id == localPlayer.id) color = IM_COL32(0, 128, 255, 255); // local = blue
        else color = IM_COL32(200, 200, 200, 255);

        // TODO: Radar projection
        float dotX = radarX + radarSize / 2; 
        float dotY = radarY + radarSize / 2;
        drawList->AddCircleFilled(ImVec2(dotX, dotY), 3, color);
    }
}

void RunNoRecoil() {
    if (!noRecoil) return;
    // TODO: Patch recoil values
}

void RunAimbot() {
    if (!aimbotEnabled) return;

    Player* target = nullptr;
    float closestDist = FLT_MAX;

    for (auto& p : players) {
        if (!IsEnemy(p)) continue;

        float dist = 0; // TODO: distance from crosshair to player
        if (dist < closestDist) {
            closestDist = dist;
            target = &p;
        }
    }

    if (target) {
        if (perfectTracking) {
            // TODO: Direct snap aim
        } else {
            float smooth = (11 - aimbotStrength) * 0.1f; // smooth factor
            // TODO: Smooth aim
        }
    }
}

// =====================================================
// GUI
// =====================================================
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

    ImGui::Text("Enemies = Red | Teammates = Green | You = Blue");
    ImGui::Text("Press INSERT to toggle menu | END to exit");
    ImGui::End();
}

void RenderKeyEntry() {
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

// =====================================================
// Main thread
// =====================================================
DWORD WINAPI MainThread(LPVOID param) {
    DecryptKey();
    while (running) {
        if (GetAsyncKeyState(VK_END) & 1) running = false;

        if (!authorized) {
            RenderKeyEntry();
        } else {
            if (GetAsyncKeyState(VK_INSERT) & 1) showMenu = !showMenu;
            if (showMenu) RenderMenu();

            // Run features
            RunESP();
            RunSkeletonESP();
            RunRadar();
            RunNoRecoil();
            RunAimbot();
        }
        Sleep(10);
    }
    FreeLibraryAndExitThread((HMODULE)param, 0);
    return 0;
}

// =====================================================
// DLL Entry
// =====================================================
BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    if (ul_reason_for_call == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hModule);
        CreateThread(nullptr, 0, MainThread, hModule, 0, nullptr);
    }
    return TRUE;
}