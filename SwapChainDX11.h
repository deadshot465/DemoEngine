#pragma once
#include "ISwapChainDX.h"
#include <d3d11_4.h>

namespace DX
{
	namespace DX11
	{
		class SwapChain :
			public ISwapChain
		{
		public:
			SwapChain(IDXGIFactory2* factory, ID3D11Device* device, HWND hwnd, int width, int height, bool tearingSupport = true);
			~SwapChain();

			// ISwapChain ÇâÓÇµÇƒåpè≥Ç≥ÇÍÇ‹ÇµÇΩ
			virtual void Present(bool vsync) const override;

			ID3D11RenderTargetView1* GetRtv() const noexcept { return mRtv.Get(); }
			ID3D11DepthStencilView* GetDsv() const noexcept { return mDsv.Get(); }

		private:
			void CreateDepthStencilView(ID3D11Device* device, int width, int height);

			Microsoft::WRL::ComPtr<ID3D11RenderTargetView1> mRtv = nullptr;
			Microsoft::WRL::ComPtr<ID3D11DepthStencilView> mDsv = nullptr;
		};
	}
}

