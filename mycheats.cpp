#include <windows.h>
#include <vector>
#include <string>
#include <cmath>
#include <iostream>

// ==============================
// GLOBALS
// ==============================
HWND hwnd = nullptr;
bool running = true;
bool menuOpen = false;

// ==============================
// PLAYER
// ==============================
struct Player {
    float x = 0, y = 0, z = 0;
    float angleX = 0, angleY = 0;
    int health = 100;
    int points = 0;
} player;

// ==============================
// ZOMBIE
// ==============================
struct Zombie {
    float x, y, z;
    float speed = 2.0f;
    int health = 50;
    bool alive = true;
};
std::vector<Zombie> zombies;

// ==============================
// WEAPON
// ==============================
struct Weapon {
    std::string name;
    int damage;
    int ammo;
    float fireRate;
    bool automatic;
    float recoil;
};
Weapon pistol  = {"Pistol", 15, 12, 2.0f, false, 0.2f};
Weapon ar      = {"Assault Rifle", 10, 25, 8.0f, true, 0.5f};
Weapon* equipped[2] = { &pistol, &ar };
int activeWeapon = 0;

// ==============================
// CHEATS
// ==============================
struct Cheats {
    bool aimbot = false;
    bool silentAim = false;
    bool fovCircle = false;
    bool noRecoil = false;
    float aimbotFov = 150.0f;
    float aimbotSmoothing = 5.0f;
} cheats;

// ==============================
// WAVE COUNTER
// ==============================
int wave = 1;

// ==============================
// INPUT
// ==============================
void UpdateInput() {
    if (GetAsyncKeyState(VK_INSERT) & 1) menuOpen = !menuOpen;
    if (GetAsyncKeyState('W') & 0x8000) player.z += 5.0f * 0.016f;
    if (GetAsyncKeyState('S') & 0x8000) player.z -= 5.0f * 0.016f;
    if (GetAsyncKeyState('A') & 0x8000) player.x -= 5.0f * 0.016f;
    if (GetAsyncKeyState('D') & 0x8000) player.x += 5.0f * 0.016f;
    if (GetAsyncKeyState('1') & 1) activeWeapon = 0;
    if (GetAsyncKeyState('2') & 1) activeWeapon = 1;

    // Menu controls
    if (menuOpen) {
        if (GetAsyncKeyState('Q') & 1) cheats.aimbot = !cheats.aimbot;
        if (GetAsyncKeyState('E') & 1) cheats.silentAim = !cheats.silentAim;
        if (GetAsyncKeyState('R') & 1) cheats.noRecoil = !cheats.noRecoil;
        if (GetAsyncKeyState('F') & 1) cheats.fovCircle = !cheats.fovCircle;

        if (GetAsyncKeyState(VK_UP) & 1) cheats.aimbotFov += 10.0f;
        if (GetAsyncKeyState(VK_DOWN) & 1) cheats.aimbotFov -= 10.0f;
        if (GetAsyncKeyState(VK_RIGHT) & 1) cheats.aimbotSmoothing += 1.0f;
        if (GetAsyncKeyState(VK_LEFT) & 1) cheats.aimbotSmoothing -= 1.0f;

        if (cheats.aimbotFov < 10) cheats.aimbotFov = 10;
        if (cheats.aimbotSmoothing < 1) cheats.aimbotSmoothing = 1;
    }
}

// ==============================
// SEMI-RAGE AIMBOT
// ==============================
void UpdateAimbot(float dt) {
    if (!cheats.aimbot || zombies.empty()) return;

    Zombie* target = nullptr;
    float minDist = cheats.aimbotFov;

    for (auto &z : zombies) {
        if (!z.alive) continue;
        float dx = z.x - player.x;
        float dz = z.z - player.z;
        float angleToZombie = atan2(dz, dx) * 180.0f / 3.14159f;
        float diff = fabs(angleToZombie - player.angleY);
        if (diff < minDist) {
            minDist = diff;
            target = &z;
        }
    }

    if (target) {
        float dx = target->x - player.x;
        float dz = target->z - player.z;
        float targetAngle = atan2(dz, dx) * 180.0f / 3.14159f;
        player.angleY += (targetAngle - player.angleY) / cheats.aimbotSmoothing;
    }
}

// ==============================
// SHOOTING
// ==============================
void Shoot() {
    Weapon* w = equipped[activeWeapon];
    static DWORD lastShotTime = 0;
    DWORD now = GetTickCount();
    float msPerShot = 1000.0f / w->fireRate;

    if (now - lastShotTime >= msPerShot && w->ammo > 0) {
        w->ammo--;
        lastShotTime = now;

        if (!cheats.noRecoil)
            player.angleX -= w->recoil;

        for (auto &z : zombies) {
            if (!z.alive) continue;

            bool hit = false;
            if (cheats.silentAim) {
                hit = true; // always hit
            } else {
                float dx = z.x - player.x;
                float dz = z.z - player.z;
                float dist = sqrtf(dx * dx + dz * dz);
                if (dist < 2.0f) hit = true;
            }

            if (hit) {
                z.health -= w->damage;
                if (z.health <= 0) {
                    z.alive = false;
                    player.points += 10;
                }
            }
        }
    }
}

// ==============================
// ZOMBIE UPDATE
// ==============================
void SpawnWave(int count) {
    zombies.clear();
    for (int i = 0; i < count; i++) {
        zombies.push_back({float((rand() % 20) - 10), 0, float((rand() % 20) - 10)});
    }
}
void UpdateZombies(float dt) {
    bool allDead = true;
    for (auto &z : zombies) {
        if (!z.alive) continue;
        allDead = false;
        float dx = player.x - z.x;
        float dz = player.z - z.z;
        float len = sqrtf(dx*dx + dz*dz);
        if (len > 0.01f) {
            z.x += (dx/len) * z.speed * dt;
            z.z += (dz/len) * z.speed * dt;
        }
    }
    if (allDead) {
        wave++;
        SpawnWave(wave * 3); // spawn more zombies per wave
    }
}

// ==============================
// CHEAT MENU
// ==============================
void RenderCheatMenu() {
    if (!menuOpen) return;

    std::cout << "\n=== CHEAT MENU (Insert to toggle) ===\n";
    std::cout << "[Q] Aimbot:      " << (cheats.aimbot ? "ON":"OFF") << "\n";
    std::cout << "[E] Silent Aim:  " << (cheats.silentAim ? "ON":"OFF") << "\n";
    std::cout << "[R] No Recoil:   " << (cheats.noRecoil ? "ON":"OFF") << "\n";
    std::cout << "[F] FOV Circle:  " << (cheats.fovCircle ? "ON":"OFF") << "\n";
    std::cout << "[↑/↓] Aimbot FOV:       " << cheats.aimbotFov << "\n";
    std::cout << "[←/→] Aimbot Smoothing: " << cheats.aimbotSmoothing << "\n";
    std::cout << "=====================================\n";
}

// ==============================
// HUD
// ==============================
void RenderHUD() {
    std::cout << "\n=== HUD ===\n";
    std::cout << "Health: " << player.health << "\n";
    std::cout << "Points: " << player.points << "\n";
    std::cout << "Weapon: " << equipped[activeWeapon]->name 
              << " | Ammo: " << equipped[activeWeapon]->ammo << "\n";
    std::cout << "Wave: " << wave << "\n";

    if (cheats.aimbot && cheats.fovCircle) {
        std::cout << "[FOV Circle Visible]\n";
    }
}

// ==============================
// GAME LOOP
// ==============================
void GameLoop() {
    DWORD lastTime = GetTickCount();

    while (running) {
        MSG msg;
        while (PeekMessage(&msg,NULL,0,0,PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            if (msg.message==WM_QUIT) running=false;
        }

        DWORD now = GetTickCount();
        float dt = (now - lastTime)/1000.0f;
        lastTime = now;

        UpdateInput();
        UpdateAimbot(dt);
        UpdateZombies(dt);

        if (GetAsyncKeyState(VK_LBUTTON) & 0x8000) Shoot();

        system("cls");
        RenderHUD();
        RenderCheatMenu();

        Sleep(16);
    }
}

// ==============================
// ENTRY POINT
// ==============================
int main() {
    srand(GetTickCount());
    SpawnWave(3); // first wave
    GameLoop();
    return 0;
}