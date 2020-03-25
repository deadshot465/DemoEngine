#include "SwapChainDX11.h"
#include "DXHelper.h"
#include "SwapChainDX12.h"

using namespace Microsoft::WRL;

DX::DX11::SwapChain::SwapChain(IDXGIFactory2* factory, ID3D11Device* device, HWND hwnd, int width, int height, bool tearingSupport)
	: ISwapChain(hwnd, width, height, tearingSupport)
{
	IDXGISwapChain1* swap_chain = nullptr;
	auto res = factory->CreateSwapChainForHwnd(device, hwnd, &mSwapChainDesc, nullptr, nullptr,
		&swap_chain);
	ThrowIfFailed(res, "Failed to create the swap chain.\n");

	res = swap_chain->QueryInterface(IID_PPV_ARGS(&mSwapChain));
	ThrowIfFailed(res, "Failed to query the swap chain.\n");

	swap_chain->Release();

	ComPtr<ID3D11Texture2D1> back_buffer = nullptr;
	res = mSwapChain->GetBuffer(0u, IID_PPV_ARGS(&back_buffer));
	ThrowIfFailed(res, "Failed to get the back buffer.\n");

	ID3D11RenderTargetView* rtv = nullptr;
	res = device->CreateRenderTargetView(back_buffer.Get(), nullptr, &rtv);
	ThrowIfFailed(res, "Failed to create render target view.\n");
	res = rtv->QueryInterface(IID_PPV_ARGS(&mRtv));
	ThrowIfFailed(res, "Failed to query render target view.\n");

	rtv->Release();
	back_buffer.Reset();

	CreateDepthStencilView(device, width, height);
}

DX::DX11::SwapChain::~SwapChain()
{
	mDsv.Reset();
	mRtv.Reset();
}

void DX::DX11::SwapChain::Present(bool vsync) const
{
	UINT flags = mTearingSupport ? DXGI_PRESENT_ALLOW_TEARING : NULL;
	UINT sync_interval = (vsync && !mTearingSupport) ? 1u : 0u;

	mSwapChain->Present(sync_interval, flags);
}

void DX::DX11::SwapChain::CreateDepthStencilView(ID3D11Device* device, int width, int height)
{
	auto desc = D3D11_TEXTURE2D_DESC1();
	desc.ArraySize = 1u;
	desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	desc.CPUAccessFlags = NULL;
	desc.Format = DXGI_FORMAT_D32_FLOAT;
	desc.Height = static_cast<UINT>(height);
	desc.MipLevels = 0u;
	desc.MiscFlags = NULL;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.TextureLayout = D3D11_TEXTURE_LAYOUT_UNDEFINED;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.Width = static_cast<UINT>(width);

	ID3D11Texture2D1* texture = nullptr;
	ComPtr<ID3D11Device3> _device = nullptr;
	auto res = device->QueryInterface(IID_PPV_ARGS(&_device));
	res = _device->CreateTexture2D1(&desc, nullptr, &texture);
	ThrowIfFailed(res, "Failed to create the texture.\n");

	res = device->CreateDepthStencilView(texture, nullptr, mDsv.ReleaseAndGetAddressOf());
	ThrowIfFailed(res, "Failed to create depth stencil view.\n");

	texture->Release();
	_device.Reset();
}
