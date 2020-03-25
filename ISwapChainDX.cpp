#include "ISwapChainDX.h"
#include "DXHelper.h"

DX::ISwapChain::ISwapChain(HWND hwnd, int32_t width, int32_t height, bool tearingSupport)
	: mTearingSupport(tearingSupport)
{
	ZeroMemory(&mSwapChainDesc, sizeof(mSwapChainDesc));
	mSwapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
	mSwapChainDesc.BufferCount = BUFFER_COUNT;
	mSwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	mSwapChainDesc.Flags = tearingSupport ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : NULL;
	mSwapChainDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	mSwapChainDesc.Height = static_cast<UINT>(height);
	mSwapChainDesc.SampleDesc.Count = 1;
	mSwapChainDesc.SampleDesc.Quality = 0;
	mSwapChainDesc.Scaling = DXGI_SCALING_STRETCH;
	mSwapChainDesc.Stereo = FALSE;
	mSwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	mSwapChainDesc.Width = static_cast<UINT>(width);
}

DX::ISwapChain::~ISwapChain()
{
	mSwapChain.Reset();
}
