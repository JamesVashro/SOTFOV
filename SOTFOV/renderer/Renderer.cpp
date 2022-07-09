#include "Renderer.h"

namespace frick {
	Renderer::Renderer() {
		renderer = this;
	}
	Renderer::~Renderer() {
		renderer = nullptr;
	}
	bool Renderer::getSwapChain(IDXGISwapChain** swapChain, ID3D11Device** device) {
		DXGI_SWAP_CHAIN_DESC desc;
		ZeroMemory(&desc, sizeof(desc));
		desc.BufferCount = 1;
		desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		desc.OutputWindow = GetForegroundWindow();
		desc.SampleDesc.Count = 1;
		desc.Windowed = TRUE;
		desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

		bool succ = false;

		std::cout << "Creating Device" << std::endl;
		HRESULT res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, 0, nullptr, 0, D3D11_SDK_VERSION,
			&desc, swapChain, device, nullptr, nullptr);

		std::cout << "Result: " << res << std::endl;

		if (SUCCEEDED(res)) {
			succ = true;
			std::cout << "Driver Type: D3D_DRIVER_TYPE_WARP ----Succeeded" << std::endl;
			std::cout << "Created Device" << std::endl;

		}
		else {
			std::cout << "Driver Type: D3D_DRIVER_TYPE_WARP ----Failed" << std::endl;
			std::cout << "Couldnt Create Device" << std::endl;
			return false;
		}

		if (!succ)
			return false;

		return true;
	}
	ID3DPresent Renderer::getPresent() {
		IDXGISwapChain* swapChain;
		ID3D11Device* device;

		if (getSwapChain(&swapChain, &device)) {

			void** vTable = *(void***)swapChain;

			if (swapChain) { swapChain->Release(); swapChain = nullptr; }
			if (device) { device->Release(); device = nullptr; }

			return (ID3DPresent)vTable[8];
		}

		return NULL;
	}
}