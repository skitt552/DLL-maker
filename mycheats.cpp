// mycheats.cpp
#include <windows.h>
#include <d3d9.h>

// ImGui core
#include "imgui.h"

// ImGui backends (for DX9 + Win32)
#include "imgui_impl_dx9.h"
#include "imgui_impl_win32.h"

struct Config {
    bool aimbot = true;
    float aimFov = 120.0f;
    float aimSmooth = 5.0f;

    bool espBox = true;
    bool espSkeleton = true;
    ImVec4 espColor = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);

    bool radar = true;
    bool triggerbot = false;
} config;

bool menuOpen = true;

void RenderESP(ImDrawList* drawList) {
    if (config.espBox)
        drawList->AddRect(ImVec2(200,200), ImVec2(260,320), ImColor(config.espColor));

    if (config.espSkeleton)
        drawList->AddLine(ImVec2(230,200), ImVec2(230,320), ImColor(config.espColor), 2.0f);
}

void RenderRadar(ImDrawList* drawList) {
    if (!config.radar) return;

    ImVec2 radarPos = ImVec2(100, 100);
    float radarSize = 120;
    drawList->AddRect(radarPos, ImVec2(radarPos.x + radarSize, radarPos.y + radarSize), IM_COL32(255,255,255,255));
    drawList->AddCircleFilled(ImVec2(radarPos.x + radarSize/2, radarPos.y + radarSize/2), 3, IM_COL32(255,0,0,255));
}

void RenderAimbotFov(ImDrawList* drawList, ImVec2 screenSize) {
    if (!config.aimbot) return;

    ImVec2 center = ImVec2(screenSize.x/2, screenSize.y/2);
    drawList->AddCircle(center, config.aimFov, IM_COL32(0,255,0,150), 64, 2.0f);
}

void RenderMenu() {
    if (!menuOpen) return;

    ImGui::Begin("Cheat Menu");

    ImGui::Checkbox("Aimbot", &config.aimbot);
    if (config.aimbot) {
        ImGui::SliderFloat("FOV", &config.aimFov, 10.0f, 360.0f);
        ImGui::SliderFloat("Smooth", &config.aimSmooth, 1.0f, 20.0f);
    }

    ImGui::Checkbox("Box ESP", &config.espBox);
    ImGui::Checkbox("Skeleton ESP", &config.espSkeleton);
    ImGui::ColorEdit4("ESP Color", (float*)&config.espColor);

    ImGui::Checkbox("2D Radar", &config.radar);
    ImGui::Checkbox("Triggerbot", &config.triggerbot);

    ImGui::End();
}

DWORD WINAPI HackThread(HMODULE hModule) {
    while (true) {
        if (GetAsyncKeyState(VK_INSERT) & 1) {
            menuOpen = !menuOpen;
        }

        ImGuiIO& io = ImGui::GetIO();
        ImDrawList* drawList = ImGui::GetBackgroundDrawList();

        RenderESP(drawList);
        RenderRadar(drawList);
        RenderAimbotFov(drawList, io.DisplaySize);

        if (menuOpen) RenderMenu();

        Sleep(16);
    }

    FreeLibraryAndExitThread(hModule, 0);
    return 0;
}

BOOL WINAPI DllMain(HMODULE hModule, DWORD dwReason, LPVOID) {
    if (dwReason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hModule);
        CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)HackThread, hModule, 0, nullptr);
    }
    return TRUE;
}