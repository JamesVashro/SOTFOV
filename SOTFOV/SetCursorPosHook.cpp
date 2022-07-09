#include "Hooking.h"
#include "menu/menu.h"
#include <iostream>

namespace frick {
	BOOL WINAPI Hooks::SetCursorPosHook(int X, int Y) {
		if (menu.opened) return FALSE;

		return hooking->SetCursorPosOriginal(X, Y);
	}
}

