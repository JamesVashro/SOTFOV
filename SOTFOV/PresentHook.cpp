#include "Hooking.h"
#include <windows.h>

#include <iostream>

#include <imgui/imgui.h>
#include <imgui/imgui_impl_dx11.h>
#include <imgui/imgui_impl_win32.h>

#include "menu/menu.h"

namespace frick {
	HRESULT Hooks::PresentHook(IDXGISwapChain* __this, UINT sync, UINT flags) {
		if (!menu.initialized) {

			//For some reason this needs to be here. if this line is not here the FOV works but the menu never gets initialized... beats me as to why but it is what it is
			std::cout << "Menu not initialized" << std::endl;

			renderer->swapChain = __this;
			__this->GetDevice(__uuidof(ID3D11Device), reinterpret_cast<PVOID*>(&renderer->device));
			renderer->device->GetImmediateContext(&renderer->context);

			DXGI_SWAP_CHAIN_DESC desc;
			__this->GetDesc(&desc);

			ImGui::CreateContext();
			ImGuiIO& io = ImGui::GetIO(); (void)io;
			io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
			io.IniFilename = NULL;
			renderer->window = desc.OutputWindow;

			ImGui_ImplWin32_Init(renderer->window);
			ImGui_ImplDX11_Init(renderer->device, renderer->context);
			ImGui::GetIO().ImeWindowHandle = renderer->window;
			ImGui_ImplDX11_CreateDeviceObjects();

			//vars->drawList = ImGui::GetBackgroundDrawList();
			ID3D11Texture2D* backBuffer;

			ImGuiStyle& style = ImGui::GetStyle();

			style.Colors[ImGuiCol_WindowBg] = ImColor(12, 12, 12, 240);
			style.Colors[ImGuiCol_TitleBg] = ImColor(12, 12, 12);
			style.Colors[ImGuiCol_TitleBgActive] = ImColor(12, 12, 12);

			style.Colors[ImGuiCol_Button] = ImColor(255, 255, 255);
			style.Colors[ImGuiCol_ButtonActive] = ImColor(0, 255, 0);
			style.Colors[ImGuiCol_ButtonHovered] = ImColor(155, 155, 155);

			style.Colors[ImGuiCol_HeaderHovered] = ImColor(80, 80, 80);
			style.Colors[ImGuiCol_HeaderActive] = ImColor(100, 100, 100);
			style.Colors[ImGuiCol_Header] = ImColor(41, 41, 41);

			style.Colors[ImGuiCol_FrameBg] = ImColor(41, 41, 41);
			style.Colors[ImGuiCol_FrameBgHovered] = ImColor(80, 80, 80);
			style.Colors[ImGuiCol_FrameBgActive] = ImColor(80, 80, 80);

			__this->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBuffer);
			renderer->device->CreateRenderTargetView(backBuffer, NULL, &renderer->targetView);
			backBuffer->Release();

			menu.Init();

			hooking->original_wndproc = reinterpret_cast<WNDPROC>(SetWindowLongPtr(renderer->window, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(WndProc_Hook)));

			menu.initialized = true;
		}

		if (menu.opened) {
			ImGui_ImplWin32_NewFrame();
			ImGui_ImplDX11_NewFrame();

			ImGui::NewFrame();


			menu.Loop();

			ImGui::EndFrame();
			ImGui::Render();

			renderer->context->OMSetRenderTargets(1, &renderer->targetView, NULL);
			ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
		}
		
		
		return hooking->original_present(__this, sync, flags);
	}
}