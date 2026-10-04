#include "pch.h"
#include "MinHook.h"

#include "curl/curl.h"
#include "Globals.h"
#include "Utils.h"
#include "Inventory.h"
#include "GameMode.h"
#include "Abilities.h"
#include "Player.h"
#include "Misc.h"
#include "Looting.h"
#include "Vehicles.h"
#include "Bots.h"
#include "Building.h"
#include "Teams.h"
#include "XP.h"
#include "Backend.h"

void InGameHooks()
{
    GameMode::HookPost();
    Misc::HookPost();
    Looting::HookPost();
    Inventory::HookFunctions();
    XP::HookFunctions();
    Bots::HookFunctions();
    Vehicles::HookFunctions();
    Abilities::HookFunctions();
    Teams::HookFunctions();
    Player::HookFunctions();
    Building::HookFunctions();

    MinHook::MH_EnableHook(MH_ALL_HOOKS);
}

void Main()
{
    AllocConsole();

    FILE* f;
    freopen_s(&f, "CONOUT$", "w", stdout);
    freopen_s(&f, "CONIN$", "r", stdin);

    // Sleep(2500); // uncomment & change if you crash on gs init

    SetConsoleTitleA("Fate 13.40 | Hooking");

    MinHook::MH_Initialize();

    GameMode::HookFunctions();
    Misc::HookFunctions();
    Looting::HookFunctions();

    MinHook::MH_EnableHook(MH_ALL_HOOKS);

 
    if (Globals::bDisableLogs)
    {
        UKismetSystemLibrary::ExecuteConsoleCommand(
            UWorld::Get(),
            L"log global warning off",
            nullptr
        );

        UKismetSystemLibrary::ExecuteConsoleCommand(
            UWorld::Get(),
            L"log global display off",
            nullptr
        );

        UKismetSystemLibrary::ExecuteConsoleCommand(
            UWorld::Get(),
            L"log global log off",
            nullptr
        );

        UKismetSystemLibrary::ExecuteConsoleCommand(
            UWorld::Get(),
            L"log global verbose off",
            nullptr
        );

        UKismetSystemLibrary::ExecuteConsoleCommand(
            UWorld::Get(),
            L"log global veryverbose off",
            nullptr
        );
    }

    Backend::Setup();

    SetConsoleTitleA("Fate 13.40 | Loading terrain");

    *(bool*)GIsClient = false;

    UWorld::Get()->OwningGameInstance->LocalPlayers.Remove(0);

    UKismetSystemLibrary::ExecuteConsoleCommand(
        UWorld::Get(),
        Globals::bCustomMap
        ? L"open /Game/Lunar/Maps/Freebuild"
        : L"open Apollo_Terrain",
        nullptr
    );
}

BOOL APIENTRY DllMain(
    HMODULE hModule,
    DWORD ul_reason_for_call,
    LPVOID lpReserved
)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        thread(Main).detach();
        break;
    }

    return TRUE;
}