#pragma once
#include "ISwapChainDX.h"
#include <d3d12.h>
#include <dxgi1_6.h>

namespace DX
{
	namespace DX12
	{
		class SwapChain :
			public DX::ISwapChain
		{
		public:
			SwapChain(IDXGIFactory2* factory, ID3D12CommandQueue* commandQueue, HWND hwnd, int32_t width, int32_t height, bool tearingSupport = true);
			~SwapChain();

			// ISwapChain ÇâÓÇµÇƒåpè≥Ç≥ÇÍÇ‹ÇµÇΩ
			virtual void Present(bool vsync) const override;
		};
	}
}

