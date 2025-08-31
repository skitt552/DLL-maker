#include <windows.h>
#include <tlhelp32.h>
#include <d3d9.h>
#include "imgui.h"
#include "imgui_impl_dx9.h"
#include "imgui_impl_win32.h"
#include <vector>
#include <string>
#include <ctime>
#include <float.h>
#include <psapi.h>

#pragma comment(lib, "d3d9.lib")

// ==========================
// Globals
// ==========================
bool showMenu = false;
bool running = true;
bool authorized = false;
bool showLog = true;

std::vector<std::string> logMessages;

// Features
bool espEnabled = false;
bool skeletonESP = false;
bool aimbotEnabled = false;
bool radarEnabled = false;
bool noRecoil = false;
bool perfectTracking = false;
int aimbotStrength = 5; // 1 = snappy, 10 = smooth

// ==========================
// Authorization key obfuscation
// ==========================
constexpr int OBF_SEED = (__TIME__[6] * __TIME__[7]) % 200 + 25;
const unsigned char obfPart1[] = { '0' ^ OBF_SEED, 0 };
const unsigned char obfPart2[] = { '2' ^ (OBF_SEED + 11), 0 };
const unsigned char obfPart3[] = { '1' ^ (OBF_SEED - 7), '1' ^ (OBF_SEED ^ 3), 0 };
char correctKey[5]; // holds "0211"

// ==========================
// Player structures (stubs)
// ==========================
struct Player { int id; bool isAlive; int teamID; float x,y,z; };
Player localPlayer;
std::vector<Player> players;

// ==========================
// Helpers
// ==========================
void AddLog(const std::string& msg) {
    logMessages.push_back(msg);
    if (logMessages.size() > 50) logMessages.erase(logMessages.begin());
}

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

// ==========================
// Features (stubs)
// ==========================
void RunESP() {}
void RunSkeletonESP() {}
void RunRadar() {}
void RunNoRecoil() {}
void RunAimbot() {}

// ==========================
// GUI
// ==========================
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
        if (strcmp(inputKey, correctKey) == 0) {
            authorized = true;
            AddLog("✅ Access Granted");
        } else {
            AddLog("❌ Wrong Key, Self Destruct!");
            SelfDestruct();
        }
    }
    ImGui::End();
}

// ==========================
// Render log overlay
// ==========================
void RenderLogOverlay() {
    if (!showLog) return;

    ImGui::Begin("DLL Log", &showLog, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize);
    for (const auto& msg : logMessages) {
        ImGui::TextUnformatted(msg.c_str());
    }
    ImGui::End();
}

// ==========================
// Main cheat thread
// ==========================
DWORD WINAPI MainThread(LPVOID param) {
    DecryptKey();
    AddLog("DLL loaded. Waiting for authorization...");
    Sleep(1000); // Wait for DX9/ImGui initialization

    while (running) {
        if (GetAsyncKeyState(VK_END) & 1) running = false;
        if (GetAsyncKeyState(VK_INSERT) & 1) showMenu = !showMenu;

        ImGui_ImplDX9_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        if (!authorized) RenderKeyEntry();
        else if (showMenu) RenderMenu();

        if (authorized) {
            RunESP();
            RunSkeletonESP();
            RunRadar();
            RunNoRecoil();
            RunAimbot();
        }

        RenderLogOverlay();

        ImGui::EndFrame();
        ImGui::Render();
        ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());

        Sleep(10);
    }

    FreeLibraryAndExitThread((HMODULE)param, 0);
    return 0;
}

// ==========================
// Process check
// ==========================
bool IsTargetProcess(const std::wstring& targetExe) {
    wchar_t path[MAX_PATH] = {0};
    if (GetModuleFileNameExW(GetCurrentProcess(), nullptr, path, MAX_PATH)) {
        std::wstring s(path);
        return (s.find(targetExe) != std::wstring::npos);
    }
    return false;
}

// ==========================
// DLL Entry with auto-hook and log
// ==========================
BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    if (ul_reason_for_call == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hModule);

        CreateThread(nullptr, 0, [](LPVOID param) -> DWORD {
            HMODULE hMod = (HMODULE)param;
            const std::wstring targetProcess = L"YourProgram.exe"; // <-- set your program executable

            AddLog("DLL attached, checking process...");

            if (IsTargetProcess(targetProcess)) {
                AddLog("Target process detected! Starting cheat thread...");
                CreateThread(nullptr, 0, MainThread, hMod, 0, nullptr);
                return 0;
            }

            while (!IsTargetProcess(targetProcess)) {
                Sleep(100);
            }

            AddLog("Target process started. Launching cheat thread...");
            CreateThread(nullptr, 0, MainThread, hMod, 0, nullptr);
            return 0;
        }, hModule, 0, nullptr);
    }
    return TRUE;
}