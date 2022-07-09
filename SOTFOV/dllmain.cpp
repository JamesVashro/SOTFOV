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

    float fasd = 0.f;
    std::string attachedToName = "";
    while (!GetAsyncKeyState(VK_DELETE) & 1) {
        if (!frick::vars->localPlayer->PlayerController)
            continue;
        
        frick::vars->output = "Checking if player is waiting";
        if (frick::vars->localPlayer->PlayerController->isWaiting > 2) {
            frick::vars->isOnMap = false;
            frick::vars->isOnCannon = false;

            frick::vars->output = "Player waiting, isOnMap = false";

        }

        if (!frick::vars->localPlayer->PlayerController->Character)
            continue;

        if (frick::vars->playerCharacter != frick::vars->localPlayer->PlayerController->Character)
            frick::vars->playerCharacter = frick::vars->localPlayer->PlayerController->Character;

        if (!frick::vars->playerCharacter)
            continue;

        if (frick::vars->AACharacter != frick::vars->playerCharacter)
            frick::vars->AACharacter = (AAthenaPlayerCharacter*)frick::vars->playerCharacter;

        if (!frick::vars->AACharacter)
            continue;

        UWieldedItemComponent* item = (UWieldedItemComponent*)frick::vars->AACharacter->WieldedItemComponent;

        if (frick::vars->isOnCannon)
            goto CheckMounted;
        
        if (frick::vars->AACharacter->CameraFOVWhenSprinting != frick::vars->sprintingFOV)
            frick::vars->AACharacter->CameraFOVWhenSprinting = frick::vars->sprintingFOV;

        if (!item)
            goto CheckMounted;


        if (!item->CurrentlyWieldedItem) {
            CleanupHeldItemFovs();
            ADSing = false;
            goto CheckMounted;
        }
        
        
        if (!frick::vars->HeldItem) {
            if (item->CurrentlyWieldedItem) {
                frick::vars->HeldItem = item->CurrentlyWieldedItem;
                frick::vars->HeldItemName = item->CurrentlyWieldedItem->GetName();
            }
        }

        if (frick::vars->HeldItemName.find("Spyglass") != -1 || frick::vars->HeldItemName.find("spyglass") != -1 || frick::vars->HeldItemName.find("telescope") != -1 || frick::vars->HeldItemName.find("Telescope") != -1) {
            frick::vars->output = "Holding Spyglass";

            spyGlass = (ASpyglass*)frick::vars->HeldItem;
            frick::vars->output = "Got Spyglass";

            if (!spyGlass)
                continue;

            spyGlass->InAimFOV = frick::vars->spyGlassFOV;


            frick::vars->output = "Set aim fov";

            if (spyGlass->IsAiming) {
                ADSing = true;
                continue;
            }

            if (!spyGlass->IsAiming) {
                ADSing = false;
                goto CheckMounted;
            }
        } else {
            if (!weapon) {
                if (frick::vars->HeldItemName.find("BP_wpn") != -1)
                    weapon = (AProjectileWeapon*)frick::vars->HeldItem;
                else
                    goto CheckMounted;
            }
            
            if (frick::vars->HeldItemName.find("BP_wpn_sniper_rifle") != -1) {
                frick::vars->output = "Holding Sniper";

                if (weapon->WeaponParameters.InAimFOV != frick::vars->sniperFOV - 15)
                    weapon->WeaponParameters.InAimFOV = frick::vars->sniperFOV - 15;
                
                goto CheckWeaponState;
            }

            if (frick::vars->HeldItemName.find("BP_wpn_flintlock_pistol") != -1) {
                frick::vars->output = "Holding pistol";

                if (weapon->WeaponParameters.InAimFOV != frick::vars->pistolFOV - 15)
                    weapon->WeaponParameters.InAimFOV = frick::vars->pistolFOV - 15;
                
                goto CheckWeaponState;

            }

            if (frick::vars->HeldItemName.find("BP_wpn_blunderbuss") != -1) {
                frick::vars->output = "Holding blunder";

                if (weapon->WeaponParameters.InAimFOV != frick::vars->blunderFOV - 15)
                    weapon->WeaponParameters.InAimFOV = frick::vars->blunderFOV - 15;
                
                goto CheckWeaponState;
            }

        CheckWeaponState:
            frick::vars->output = "Checking weapon state";

            switch (weapon->State.GetValue()) {
                case EProjectileWeaponState::EProjectileWeaponState__Aiming:
                    ADSing = true;
                    break;
                case EProjectileWeaponState::EProjectileWeaponState__Recoil:
                    if (frick::vars->HeldItemName.find("sniper_rifle") != -1) {
                        if (ADSing)
                            break;
                            
                    } else
                        ADSing = false;
                    break;
                case EProjectileWeaponState::EProjectileWeaponState__Reloading:
                    ADSing = false;
                    break;
                default:
                    if (ADSing)
                    ADSing = false;
                    break;
            }
        }
        
    CheckMounted:
        frick::vars->output = "Checking if mounted";

        if (frick::vars->AACharacter->IsMounted == 69) {
            if (attachedToName != "")
                attachedToName = "";

            frick::vars->isOnCannon = false;
            frick::vars->isOnMap = false;

            goto SetFOV;
        } else if (frick::vars->AACharacter->IsMounted == 77) {
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

            AActor* attachedTo = frick::vars->localPlayer->PlayerController->AcknowledgedPawn->GetAttachParentActor();

            if (attachedTo)
                if (attachedTo->GetName().find("Cannon") != -1)
                    frick::vars->AttachedCannon = (ABP_Cannon_C*)attachedTo;
            
            if (frick::vars->AttachedCannon) {
                frick::vars->output = "Attached to cannon... settings fovs";
                frick::vars->AttachedCannon->AimFOV = frick::vars->cannonFOVads - 15;
                frick::vars->AttachedCannon->DefaultFOV = frick::vars->cannonFOV;
            }

            frick::vars->playerCharacter->SetTargetFOV(frick::vars->AACharacter, frick::vars->cannonFOV);
            continue;
        }

        if (frick::vars->AACharacter->IsMounted != 79 && frick::vars->isOnCannon) {
            frick::vars->isOnCannon = false;

            if (frick::vars->AttachedCannon) {
                frick::vars->output = "Off cannon, resetting fovs";

                frick::vars->AttachedCannon->AimFOV = 70.f;
                frick::vars->AttachedCannon->DefaultFOV = 85.f;
                frick::vars->AttachedCannon = nullptr;
            }
        }

    SetFOV:
        if (!ADSing) {
            if (!frick::vars->isOnCannon) {
                if (!frick::vars->mapFOV) {
                    if (frick::vars->AACharacter->ReplicatedIsSprinting) {
                        frick::vars->output = "Setting FOV to sprinting FOV";
                        frick::vars->playerCharacter->SetTargetFOV(frick::vars->AACharacter, frick::vars->sprintingFOV);
                    }
                    else {
                        frick::vars->output = "Setting FOV to FOV";
                        frick::vars->playerCharacter->SetTargetFOV(frick::vars->AACharacter, frick::vars->FOV);
                    }
                } else {
                    if (frick::vars->isOnMap) {
                        frick::vars->output = "On map setting FOV Map FOV";
                        frick::vars->playerCharacter->SetTargetFOV(frick::vars->AACharacter, 89.f);

                    } else {
                        frick::vars->output = "On map, setting fov to fov";
                        frick::vars->playerCharacter->SetTargetFOV(frick::vars->AACharacter, frick::vars->FOV);

                    }
                }
            }
        } else {
            frick::vars->output = "ADSing with weapon: " + frick::vars->HeldItemName;
        }

        if (frick::vars->performance)
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
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

