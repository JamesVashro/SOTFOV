// dllmain.cpp : Defines the entry point for the DLL application.
#include <windows.h>
#include <iostream>
#include <thread>
#include <algorithm>
#include "MinHook.h"

#include "FindShit.h"
#include "renderer/Renderer.h"
#include "Hooking.h"
#include "SDK/SDK.h"
#include "vars.h"
#include <fstream>

#include <Windows.h>
#include <Dbghelp.h>

#pragma comment(lib, "libMinHook.x64.lib")

FILE* f;

auto renderer = std::make_unique<frick::Renderer>();
auto hooking = std::make_unique<frick::Hooking>();
auto vars = std::make_unique<frick::Vars>();

void CleanupAndShutdown(HMODULE hModule) {
    MH_Uninitialize();

    

    renderer.reset();
    hooking.reset();
    vars.reset();

    CloseHandle(hModule);
    FreeLibraryAndExitThread(hModule, 0);
}
void doThing(HMODULE hModule) {

    AllocConsole();
    freopen_s(&f, "CONOUT$", "w", stdout);
    
    frick::vars->hModule = hModule;
    frick::hooking->original_present = frick::renderer->getPresent();

    if (MH_CreateHook(frick::hooking->original_present, frick::Hooks::PresentHook, reinterpret_cast<void**>(&frick::hooking->original_present)) != MH_OK) {
        std::cout << "Couldnt create Present Hook" << std::endl;

    } else {
        if (MH_CreateHook(SetCursorPos, frick::Hooks::SetCursorPosHook, reinterpret_cast<void**>(&frick::hooking->SetCursorPosOriginal)) != MH_OK) {
            std::cout << "Couldnt create MousePos Hook" << std::endl;
        }
        else {
            if (MH_CreateHook(SetCursor, frick::Hooks::SetCursorHook, reinterpret_cast<void**>(&frick::hooking->SetCursorOriginal)) != MH_OK) {
                std::cout << "Couldnt create SetCursor Hook" << std::endl;
            } 
            else {
                if (MH_EnableHook(MH_ALL_HOOKS) != MH_OK) {
                    std::cout << "Couldnt apply hooks" << std::endl;
                    CleanupAndShutdown(hModule);
                }
            }    
        }
    }
    
    FreeConsole();
    fclose(f);
    

    if (!InitializeObjects() || !InitializeNames()) {
        CleanupAndShutdown(hModule);
    }
    
    if (FindWorld(frick::vars->GWorld)) {
        std::cout << "World: " << frick::vars->GWorld << std::endl;
        auto world = *frick::vars->GWorld;
        if (!world)
            return;

        frick::vars->localPlayer = world->OwningGameInstance->LocalPlayers[0];

        if (!frick::vars->localPlayer)
            return;

        printf("Dereferenced World: %p\n", world);
    }


    std::string attachedToName = "";
    int oldResult = 0;
    float multiplier = 1.0f;

    while (!GetAsyncKeyState(VK_DELETE) & 1) {
        if (!frick::vars->localPlayer->PlayerController)
            continue;
        
        frick::vars->playerCharacter = frick::vars->localPlayer->PlayerController->Character;

        if (!frick::vars->playerCharacter)
            continue;
        
        frick::vars->AACharacter = (AAthenaPlayerCharacter*)frick::vars->playerCharacter;

        if (!frick::vars->AACharacter)
            continue;
        
        int result = (int)frick::vars->playerCharacter->GetTargetFOV(frick::vars->AACharacter);

        if (result != 17 && result != oldResult) {
            //printf("--------------\ngetfov Result: %i          %i\n", result, oldResult);

            if (result == 78) //normal fov
                multiplier = 1.0f;
            else if (result == 73) //steering wheel, harpoon, cannon
                multiplier = 0.94444441025f;
            else if (result == 90) {//sprinting
                if (frick::vars->sprintFOV)
                    multiplier = 1.1f;
                else
                    multiplier = 1.0f;
            }
            else if (result == 60) //pistol zoom
                multiplier = 0.76923076923f;
            else if (result == 70) //blunderbuss
                multiplier = 0.89743589743f;
            else if (result == 30) //sniper
                multiplier = 0.38461538461f;

            
            //printf("Updated multiplyer: %f\nSetting FOv to: %f\n------------\n", multiplier, frick::vars->FOV * multiplier);
            frick::vars->playerCharacter->SetTargetFOV(frick::vars->AACharacter, frick::vars->FOV * multiplier);

        }

        oldResult = result;
    }
    CleanupAndShutdown(hModule);
}

BOOL APIENTRY DllMain(HMODULE hModule,
    DWORD  ul_reason_for_call,
    LPVOID lpReserved
)
{
    if (ul_reason_for_call == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hModule);
        CreateThread(0, 0, (LPTHREAD_START_ROUTINE)doThing, hModule, 0, NULL);
    }
    return TRUE;
}