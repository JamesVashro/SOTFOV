#pragma once
#include "SDK/SDK.h"
#include <fstream>
#include "imgui/ImGuiFileBrowser.h"

#include "Hooking.h"

namespace frick {
	class Vars {
		
		public:
			Vars();
			~Vars();

			static inline	HMODULE							hModule;

			static inline	std::string						cfgFileName = "";
			static inline	std::string						cfgFilePath = "";
			static inline	std::fstream					cfgFile;
			static inline	std::string						cfgFileDir;
			static inline	imgui_addons::ImGuiFileBrowser	fileBrowser;

			static inline	bool							SettingHotkey = false;
			static inline	int								HotKey = VK_INSERT;

			static inline	float							FOV = 120.f;
			static inline	float							sniperFOV = 65.f;
			static inline	float							pistolFOV = 100.f;
			static inline	float							blunderFOV = 120.f;
			static inline	float							spyGlassFOV = 43.f;
			static inline	float							sprintingFOV = 130.f;
			static inline	float							cannonFOVads = 100.f;
			static inline	float							cannonFOV = 120.f;

			static inline	float							default_FOV = 120.f;
			static inline	float							default_sniperFOV = 65.f;
			static inline	float							default_pistolFOV = 100.f;
			static inline	float							default_blunderFOV = 120.f;
			static inline	float							default_spyGlassFOV = 43.f;
			static inline	float							default_sprintingFOV = 130.f;
			static inline	float							default_cannonFOVads = 100.f;
			static inline	float							default_cannonFOV = 120.f;
			static inline	bool							default_MapFOV = false;

			static inline	float							lastResult = -1;
			static inline	bool							simpleMode = false;
			static inline	bool							cheats = false;
			static inline	bool							isOnCannon = false;
			static inline	bool							linked = false;
			
			static inline	bool							PostRenderHooked = false;
			static inline	bool							hookPostRender = false;

			static inline	bool							mapFOV = false;
			static inline	bool							isOnMap = false;

			static inline	bool							saveDialogOpen = false;
			static inline	bool							openDialogOpen = false;
			

			static inline	bool							performance = false;

			static inline	UWorld**						GWorld = nullptr;
			static inline	ACharacter*						playerCharacter{};
			static inline	AAthenaPlayerCharacter*			AACharacter{};
			static inline	ULocalPlayer*					localPlayer{};

			static inline	bool							Loading = false;

			static inline	ABP_Cannon_C*					AttachedCannon{};
			static inline	AActor*							HeldItem{};
			static inline	std::string						HeldItemName = "";

			static inline float curFOV = 0.f;
	};
	inline Vars* vars{};
}