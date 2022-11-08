#pragma once
#include "SDK/SDK.h"

#include <d3d11.h>

#include <iostream>
#pragma comment(lib, "d3d11.lib")

namespace frick {
	typedef HRESULT(__stdcall* ID3DPresent)(IDXGISwapChain* this_, UINT sync, UINT flags);
	typedef HCURSOR(__stdcall* Set_Cursor)(HCURSOR hCursor);

	class Renderer
	{
	public:
		explicit Renderer();
		~Renderer();
		bool getSwapChain(IDXGISwapChain** swapChain, ID3D11Device** device);
		ID3DPresent getPresent();

	public:
		static inline IDXGISwapChain* swapChain;
		static inline HWND window;
		static inline ID3D11Device* device;
		static inline ID3D11DeviceContext* context;
		static inline ID3D11RenderTargetView* targetView;
	private:

	};
	inline Renderer* renderer{};
}


