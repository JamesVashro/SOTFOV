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

			static inline	bool							SettingHotkey = false;
			static inline	int								HotKey = VK_INSERT;

			static inline	float							FOV = 104.f;
			static inline	bool							sprintFOV = false;

			static inline	float							lastResult = -1;

			static inline	std::string						output = "";
			static inline	bool							performance = false;

			static inline	UWorld**						GWorld = nullptr;
			static inline	ACharacter*						playerCharacter{};
			static inline	AAthenaPlayerCharacter*			AACharacter{};
			static inline	ULocalPlayer*					localPlayer{};

			static inline	bool							Loading = false;

			static inline	AActor*							HeldItem{};
			static inline	std::string						HeldItemName = "";
	};
	inline Vars* vars{};
}