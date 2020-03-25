#pragma once
#include <dxgi1_6.h>
#include <Windows.h>
#include <wrl/client.h>
#include <cstdint>

namespace DX
{
	class ISwapChain
	{
	public:
		ISwapChain(HWND hwnd, int32_t width, int32_t height, bool tearingSupport);
		virtual ~ISwapChain();

		virtual void Present(bool vsync) const = 0;

		IDXGISwapChain4* GetSwapChain() const noexcept { return mSwapChain.Get(); }

	protected:
		Microsoft::WRL::ComPtr<IDXGISwapChain4> mSwapChain = nullptr;
		DXGI_SWAP_CHAIN_DESC1 mSwapChainDesc = {};
		bool mTearingSupport = false;
	};
}

