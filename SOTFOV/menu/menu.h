#include <windows.h>
#include <fstream>
#include <algorithm>

#include <imgui/imgui.h>
#include <imgui/imgui_impl_dx11.h>
#include <imgui/imgui_impl_win32.h>
#include <imgui/ImGuiFileBrowser.h>

#include "vars.h"

namespace frick {
	class Menu {
	public:
		void Init();
		void Loop();
	public:
		bool initialized;
		bool opened;
		bool showConsole;
		bool showPlayerWindow;
		bool showGhostWindow;
		bool showHooks;
	};
	inline Menu menu;
}