#include "Hooking.h"

namespace frick {
	Hooking::Hooking() {
		MH_STATUS status = MH_Initialize();
		if (status != MH_OK) {
			//std::cout << MH_StatusToString(status) << std::endl;
			GoodToApply = false;
		}
		hooking = this;
	}

	Hooking::~Hooking() {
		SetWindowLongPtr(renderer->window, GWLP_WNDPROC, (LONG_PTR)this->original_wndproc);
		hooking = nullptr;
	}

	bool Hooking::SetHook(LPVOID func, LPVOID detour, LPVOID* original, std::string name) {
		//std::cout << "Setting hook: " << name << std::endl;

		if (MH_CreateHook(func, detour, reinterpret_cast<void**>(&original)) != MH_OK) {
			//std::cout << "Couldnt Create hook: " << name << std::endl;
			GoodToApply = false;
			return false;
		}
		return true;
	}

	bool Hooking::ApplyHooks() {
		//std::cout << "Applying Hooks" << std::endl;
		if (GoodToApply) {
			if (MH_EnableHook(MH_ALL_HOOKS) != MH_OK) {
				//std::cout << "Couldnt apply hooks" << std::endl;
				return false;
			}
		}
		return true;
	}
}