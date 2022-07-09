#include "Hooking.h"
#include <windows.h>
#include "menu/menu.h"

namespace frick {
	HCURSOR WINAPI Hooks::SetCursorHook(HCURSOR hCursor){
		if (menu.opened) return 0;

		return hooking->SetCursorOriginal(hCursor);
	}
}
