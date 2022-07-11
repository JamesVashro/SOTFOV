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
bool ADSing = false;
ASpyglass* spyGlass = nullptr;
AProjectileWeapon* weapon = nullptr;

auto renderer = std::make_unique<frick::Renderer>();
auto hooking = std::make_unique<frick::Hooking>();
auto vars = std::make_unique<frick::Vars>();

void CleanupHeldItemFovs() {
    frick::vars->output = "Cleaning up fovs";
    if (frick::vars->HeldItem) {
        if (frick::vars->HeldItemName.find("Spyglass") != -1 || frick::vars->HeldItemName.find("spyglass") != -1) {
            ASpyglass* spyGlass = (ASpyglass*)frick::vars->HeldItem;
            spyGlass->InAimFOV = 17.f;

        }
        else {
            if (frick::vars->HeldItemName.find("BP_wpn") != -1) {
                AProjectileWeapon* weapon = (AProjectileWeapon*)frick::vars->HeldItem;
                weapon->WeaponParameters.InAimFOV = frick::vars->getWeaponDefaultFOV(frick::vars->HeldItemName);
            }
        }

        frick::vars->HeldItem = nullptr;
        frick::vars->HeldItemName = "";
        weapon = nullptr;
        spyGlass = nullptr;
    }

    frick::vars->output = "Done cleaning up fovs";

}

void CleanupAndShutdown(HMODULE hModule) {
    MH_Uninitialize();

    if (frick::vars->PostRenderHooked)
        frick::vars->UnHookPostRender();

    if (frick::vars->AACharacter) {
        frick::vars->AACharacter->CameraFOVWhenSprinting = 90.5f;
    }

    CleanupHeldItemFovs();

    

    renderer.reset();
    hooking.reset();
    vars.reset();

    CloseHandle(hModule);
    FreeLibraryAndExitThread(hModule, 0);
}



void make_minidump(EXCEPTION_POINTERS* e)
{
    auto path = frick::vars->cfgFileDir + "\\CrashDump.txt";
    std::fstream file;
    file.open(path, std::ios::out);

    if (file.is_open()) {
        file << "AACharacter                    " << frick::vars->AACharacter << "\n";
        file << "PlayerCharacter                " << frick::vars->playerCharacter << "\n";
        file << "Cheats                         " << frick::vars->cheats << "\n";
        file << "Spyglass                       " << spyGlass << "\n";
        file << "Weapon                         " << weapon << "\n";
        file << "Current Output:                " << frick::vars->output << "\n";
        file << "Performance Mode:              " << frick::vars->performance << "\n";
        file << "Currently Held Item:           " << frick::vars->HeldItemName << "\n";
    }

    file.close();
    return;
}

LONG CALLBACK unhandled_handler(EXCEPTION_POINTERS* e)
{
    make_minidump(e);
    return EXCEPTION_CONTINUE_SEARCH;
}


void doThing(HMODULE hModule) {
    SetUnhandledExceptionFilter(unhandled_handler);

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

        frick::vars->ViewPortClient = frick::vars->localPlayer->ViewportClient;

        if (!frick::vars->ViewPortClient)
            return;

        printf("Dereferenced World: %p\n", world);
    }


    std::string attachedToName = "";
    while (!GetAsyncKeyState(VK_DELETE) & 1) {
        if (frick::vars->performance)
            std::this_thread::sleep_for(std::chrono::milliseconds(5));

        if (!frick::vars->localPlayer->PlayerController)
            continue;

        if (!frick::vars->localPlayer->PlayerController->Character) {
            frick::vars->isOnCannon = false;
            frick::vars->isOnMap = false;
            continue;
        }
        
        frick::vars->playerCharacter = frick::vars->localPlayer->PlayerController->Character;

        if (!frick::vars->playerCharacter)
            continue;
        
        frick::vars->AACharacter = (AAthenaPlayerCharacter*)frick::vars->playerCharacter;

        if (!frick::vars->AACharacter)
            continue;
        
        int result = (int)frick::vars->playerCharacter->GetTargetFOV(frick::vars->AACharacter);
        
        frick::vars->output = "Checking if mounted";

        if (frick::vars->AACharacter->IsMounted == 69) {
            if (attachedToName != "")
                attachedToName = "";

            frick::vars->isOnCannon = false;
            frick::vars->isOnMap = false;

            goto SetFOV;
        }
        else if (frick::vars->AACharacter->IsMounted == 77) {
            frick::vars->output = "Mounted to 79. checking if map";
            attachedToName = frick::vars->AACharacter->AttachmentReplication.AttachComponent->GetName();

            if (attachedToName == "TableMesh") {
                frick::vars->isOnMap = true;
                frick::vars->output = "Mounted To map, setting FOV";

                goto SetFOV;
            }
        }

        if (frick::vars->AACharacter->IsMounted == 79 && !frick::vars->isOnCannon) {
            frick::vars->isOnCannon = true;
            frick::vars->playerCharacter->SetTargetFOV(frick::vars->AACharacter, frick::vars->cannonFOV);
            continue;
        }


    SetFOV:
        if (frick::vars->isOnMap) {
            if (frick::vars->mapFOV)
                frick::vars->playerCharacter->SetTargetFOV(frick::vars->AACharacter, 89.f);
            else
                frick::vars->playerCharacter->SetTargetFOV(frick::vars->AACharacter, frick::vars->FOV);

            continue;
        }

        if (result == 90) {
            frick::vars->playerCharacter->SetTargetFOV(frick::vars->AACharacter, frick::vars->sprintingFOV);
            continue;
        }

        if (result == 78 || result == 73) {
            if (frick::vars->isOnCannon)
                frick::vars->playerCharacter->SetTargetFOV(frick::vars->AACharacter, frick::vars->cannonFOV);
            else
                frick::vars->playerCharacter->SetTargetFOV(frick::vars->AACharacter, frick::vars->FOV);

            continue;
        }

        if (result == 60) {
            if (frick::vars->isOnCannon)
                frick::vars->playerCharacter->SetTargetFOV(frick::vars->AACharacter, frick::vars->cannonFOVads);
            else
                frick::vars->playerCharacter->SetTargetFOV(frick::vars->AACharacter, frick::vars->pistolFOV);

            continue;
        }

        if (result == 30) {
            frick::vars->playerCharacter->SetTargetFOV(frick::vars->AACharacter, frick::vars->sniperFOV);
            continue;
        }

        if (result == 17) {
            frick::vars->playerCharacter->SetTargetFOV(frick::vars->AACharacter, frick::vars->spyGlassFOV);
            continue;
        }

        if (result == 70) {
            frick::vars->playerCharacter->SetTargetFOV(frick::vars->AACharacter, frick::vars->blunderFOV);
            continue;
        }
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

