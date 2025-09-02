#include "MinHook.h"
#include <d3d9.h>
#include <Windows.h>
#include <TlHelp32.h> // For Process Snapshot functions
#include <string>

// Link necessary libraries
#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "dwmapi.lib")
#pragma comment(lib, "d3dx9.lib")

// Function Pointers
typedef HRESULT(APIENTRY* EndScene_t)(LPDIRECT3DDEVICE9 pDevice);
EndScene_t oEndScene = nullptr;

// Global variables
bool espEnabled = true;
bool aimbotEnabled = true;
bool noRecoilEnabled = true;

uintptr_t gameModuleBase = 0;
uintptr_t entityListAddr = 0;
uintptr_t viewAnglesAddr = 0;
uintptr_t recoilAddr = 0;

// Memory helpers
uintptr_t GetModuleBase(const char* modName, DWORD procId) {
    uintptr_t modBase = 0;
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, procId);
    if (snap != INVALID_HANDLE_VALUE) {
        MODULEENTRY32 modEntry;
        modEntry.dwSize = sizeof(modEntry);
        if (Module32First(snap, &modEntry)) {
            do {
                if (!_stricmp(modEntry.szModule, modName)) {
                    modBase = (uintptr_t)modEntry.modBaseAddr;
                    break;
                }
            } while (Module32Next(snap, &modEntry));
        }
    }
    CloseHandle(snap);
    return modBase;
}

uintptr_t PatternScan(uintptr_t start, size_t size, const char* pattern, const char* mask) {
    MEMORY_BASIC_INFORMATION mbi{};
    for (uintptr_t addr = start; addr < start + size;) {
        if (!VirtualQuery((LPCVOID)addr, &mbi, sizeof(mbi))) break;
        if (mbi.State == MEM_COMMIT && (mbi.Protect & (PAGE_EXECUTE_READWRITE | PAGE_READWRITE | PAGE_EXECUTE_READ))) {
            for (size_t i = 0; i < mbi.RegionSize; i++) {
                bool found = true;
                for (size_t j = 0; mask[j]; j++) {
                    if (mask[j] != '?' && pattern[j] != *(char*)(addr + i + j)) {
                        found = false;
                        break;
                    }
                }
                if (found) return addr + i;
            }
        }
        addr += mbi.RegionSize;
    }
    return 0;
}

// ESP Drawing
void DrawTextSimple(LPDIRECT3DDEVICE9 pDevice, const char* text, int x, int y, D3DCOLOR color) {
    static ID3DXFont* g_pFont = nullptr;
    if (!g_pFont) {
        D3DXCreateFont(pDevice, 16, 0, FW_BOLD, 1, FALSE, DEFAULT_CHARSET,
            OUT_DEFAULT_PRECIS, ANTIALIASED_QUALITY,
            DEFAULT_PITCH | FF_DONTCARE, "Arial", &g_pFont);
    }

    RECT rect;
    SetRect(&rect, x, y, x + 200, y + 20);
    g_pFont->DrawTextA(nullptr, text, -1, &rect, DT_LEFT | DT_NOCLIP, color);
}

// Hooked EndScene Function
HRESULT APIENTRY hkEndScene(LPDIRECT3DDEVICE9 pDevice) {
    // ESP Drawing
    if (espEnabled && entityListAddr) {
        Entity* ents = reinterpret_cast<Entity*>(entityListAddr);
        if (ents) {
            for (int i = 0; i < 32; i++) { // Assumes max 32 entities
                if (ents[i].health > 0 && ents[i].isEnemy) {
                    char buf[64];
                    sprintf_s(buf, "Enemy HP: %d", ents[i].health);
                    DrawTextSimple(pDevice, buf, 300, 200 + (i * 15), D3DCOLOR_ARGB(255, 255, 0, 0));
                }
            }
        }
    }

    // Aimbot logic can go here...
    if (aimbotEnabled && viewAnglesAddr && entityListAddr) {
        DrawTextSimple(pDevice, "Aimbot Active", 500, 100, D3DCOLOR_ARGB(255, 0, 255, 0));
    }

    // No recoil logic
    if (noRecoilEnabled && recoilAddr) {
        DWORD oldProtect;
        if (VirtualProtect((LPVOID)recoilAddr, sizeof(float) * 2, PAGE_EXECUTE_READWRITE, &oldProtect)) {
            *(float*)(recoilAddr) = 0.0f;
            *(float*)(recoilAddr + 4) = 0.0f;
            VirtualProtect((LPVOID)recoilAddr, sizeof(float) * 2, oldProtect, &oldProtect);
            DrawTextSimple(pDevice, "No Recoil Applied", 500, 120, D3DCOLOR_ARGB(255, 0, 200, 255));
        }
    }

    return oEndScene(pDevice);
}

// Hook Setup and MainThread
DWORD WINAPI MainThread(LPVOID lpReserved) {
    if (MH_Initialize() != MH_OK) return 1;

    DWORD pid = GetCurrentProcessId();
    gameModuleBase = GetModuleBase("MyGame.exe", pid);

    // Example pattern scans (adjust based on your game)
    entityListAddr = PatternScan(gameModuleBase, 0x1000000, "\x89\x54\x24\x10\x8B\x45", "xxxxxx");
    viewAnglesAddr = PatternScan(gameModuleBase, 0x1000000, "\xF3\x0F\x11\x05\x00\x00\x00\x00", "xxxx????");
    recoilAddr = PatternScan(gameModuleBase, 0x1000000, "\xF3\x0F\x11\x86\x00\x00\x00\x00", "xxxx????");

    IDirect3D9* pD3D = Direct3DCreate9(D3D_SDK_VERSION);
    if (!pD3D) return 1;

    D3DPRESENT_PARAMETERS d3dpp = {};
    d3dpp.Windowed = TRUE;
    d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    d3dpp.hDeviceWindow = GetForegroundWindow();

    LPDIRECT3DDEVICE9 pDevice = nullptr;
    if (FAILED(pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL,
        d3dpp.hDeviceWindow,
        D3DCREATE_SOFTWARE_VERTEXPROCESSING,
        &d3dpp, &pDevice))) {
        pD3D->Release();
        return 1;
    }

    void** pVTable = *reinterpret_cast<void***>(pDevice);

    if (MH_CreateHook(pVTable[42], &hkEndScene, reinterpret_cast<void**>(&oEndScene)) != MH_OK)
        return 1;

    if (MH_EnableHook(pVTable[42])
