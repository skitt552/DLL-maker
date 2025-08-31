#include <windows.h>
#include "imgui.h"
#include "backends/imgui_impl_dx9.h"
#include "backends/imgui_impl_win32.h"

// Forward declare WndProc handler
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(
    HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Global state
HWND g_hWnd = nullptr;
LPDIRECT3DDEVICE9 g_pDevice = nullptr;
bool g_Running = true;

// Thread for our ImGui loop
DWORD WINAPI MainThread(LPVOID lpReserved)
{
    // Initialize ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui::StyleColorsDark();

    // Initialize platform/renderer backends
    ImGui_ImplWin32_Init(g_hWnd);
    ImGui_ImplDX9_Init(g_pDevice);

    // Main loop
    while (g_Running)
    {
        ImGui_ImplDX9_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        // Example window
        ImGui::Begin("Hello from ImGui v4!");
        ImGui::Text("This is a test overlay.");
        if (ImGui::Button("Exit"))
            g_Running = false;
        ImGui::End();

        // Render
        ImGui::EndFrame();
        g_pDevice->BeginScene();
        ImGui::Render();
        ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
        g_pDevice->EndScene();

        Sleep(10);
    }

    // Cleanup
    ImGui_ImplDX9_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    FreeLibraryAndExitThread((HMODULE)lpReserved, 0);
    return 0;
}

// DllMain entry point
BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    if (ul_reason_for_call == DLL_PROCESS_ATTACH)
    {
        DisableThreadLibraryCalls(hModule);
        CreateThread(nullptr, 0, MainThread, hModule, 0, nullptr);
    }
    return TRUE;
}