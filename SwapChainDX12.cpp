#include "SwapChainDX12.h"
#include "DXHelper.h"

DX::DX12::SwapChain::SwapChain(IDXGIFactory2* factory, ID3D12CommandQueue* commandQueue, HWND hwnd, int32_t width, int32_t height, bool tearingSupport)
	: ISwapChain(hwnd, width, height, tearingSupport)
{
	IDXGISwapChain1* swap_chain = nullptr;
	auto res = factory->CreateSwapChainForHwnd(commandQueue, hwnd, &mSwapChainDesc, nullptr, nullptr, &swap_chain);
	ThrowIfFailed(res, "Failed to create swap chain.\n");

	res = swap_chain->QueryInterface(IID_PPV_ARGS(&mSwapChain));
	ThrowIfFailed(res, "Failed to query swap chain.\n");

	swap_chain->Release();
}

DX::DX12::SwapChain::~SwapChain()
{

}

void DX::DX12::SwapChain::Present(bool vsync) const
{
	UINT flags = mTearingSupport ? DXGI_PRESENT_ALLOW_TEARING : NULL;
	UINT sync_interval = (vsync && !mTearingSupport) ? 1u : 0u;
	mSwapChain->Present(sync_interval, flags);
}