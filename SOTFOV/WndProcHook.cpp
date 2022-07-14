#include "Hooking.h"

#include "menu/menu.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace frick {
	LRESULT WINAPI Hooks::WndProc_Hook(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {


		if (msg == WM_KEYUP && wparam == vars->HotKey) {
			menu.opened = !menu.opened;
		}

		if (menu.opened) {
			ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam);
            


            if (vars->SettingHotkey) {
                if (wparam == VK_F1 || wparam == VK_F2 || wparam == VK_F3 || wparam == VK_F4 || wparam == VK_F5 || wparam == VK_F6 ||
                    wparam == VK_F7 || wparam == VK_F8 || wparam == VK_F9 || wparam == VK_F11 || wparam == VK_F12 ||
                    wparam == VK_INSERT || wparam == VK_HOME || wparam == VK_END
                    || wparam == VK_PAUSE || wparam == VK_SCROLL) {

                    vars->HotKey = (int)wparam;
                    printf("%i\n", (int)wparam);
                    vars->SettingHotkey = false;
                }
            }

            ImGuiMouseCursor imgui_cursor = ImGui::GetMouseCursor();
            LPCSTR win32_cursor = IDC_ARROW;
            switch (imgui_cursor)
            {
                case ImGuiMouseCursor_Arrow:        win32_cursor = IDC_ARROW; break;
                case ImGuiMouseCursor_TextInput:    win32_cursor = IDC_IBEAM; break;
                case ImGuiMouseCursor_ResizeAll:    win32_cursor = IDC_SIZEALL; break;
                case ImGuiMouseCursor_ResizeEW:     win32_cursor = IDC_SIZEWE; break;
                case ImGuiMouseCursor_ResizeNS:     win32_cursor = IDC_SIZENS; break;
                case ImGuiMouseCursor_ResizeNESW:   win32_cursor = IDC_SIZENESW; break;
                case ImGuiMouseCursor_ResizeNWSE:   win32_cursor = IDC_SIZENWSE; break;
                case ImGuiMouseCursor_Hand:         win32_cursor = IDC_HAND; break;
                case ImGuiMouseCursor_NotAllowed:   win32_cursor = IDC_NO; break;
            }
            hooking->SetCursorOriginal(LoadCursorA(nullptr, win32_cursor));

			return true;
		}

		return CallWindowProc(hooking->original_wndproc, hwnd, msg, wparam, lparam);
	}
}