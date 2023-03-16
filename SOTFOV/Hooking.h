#pragma once
#include <windows.h>
#include <detours/detours.h>
#include <dxgi.h>
#include <string>
#include <vector>

#include "MinHook.h"
#include "renderer/Renderer.h"
#include "SDK/SDK.h"

namespace frick {
	struct Hooks {
		static HRESULT PresentHook(IDXGISwapChain* swapChain, UINT sync, UINT flags);
		static LRESULT WINAPI WndProc_Hook(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
		static HCURSOR WINAPI SetCursorHook(HCURSOR hCursor);
		static BOOL WINAPI SetCursorPosHook(int X, int Y);
		static void ProcessEventHook(void* obj, UFunction* func, void* params);
	};

	class Hooking {
		friend Hooks;
	public:
		explicit Hooking();
		~Hooking();
		bool SetHook(LPVOID func, LPVOID detour, LPVOID* original, std::string name);
		bool ApplyHooks();
	public:
		ID3DPresent original_present{};
		WNDPROC original_wndproc{};
		ProcessEvent oProcessEvent{};
		static inline decltype(SetCursorPos)* SetCursorPosOriginal = nullptr;
		static inline decltype(SetCursor)* SetCursorOriginal = nullptr;

		bool GoodToApply;
		PostRender oPostRender{};
	};
	inline Hooking* hooking{};
}



