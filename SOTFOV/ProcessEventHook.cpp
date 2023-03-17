#include "Hooking.h"
#include "vars.h"

namespace frick {
	void Hooks::ProcessEventHook(void* obj, UFunction* func, void* params) {
		
		if (func)
		{
			if (func->GetName().find("RadialBlurOn") != -1)
				return;
		}

		return hooking->oProcessEvent(obj, func, params);
	}
}