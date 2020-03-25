#include "DeviceContext.h"
#include <utility>
#include "DXHelper.h"

DX::DX11::DeviceContext::DeviceContext(Microsoft::WRL::ComPtr<ID3D11DeviceContext4>& deviceContext)
	: mDeviceContext(std::move(deviceContext))
{
}

DX::DX11::DeviceContext::~DeviceContext()
{
	mDeviceContext.Reset();
}

void DX::DX11::DeviceContext::Clear(const std::array<float, 4>& color, ID3D11RenderTargetView* rtv, ID3D11DepthStencilView* dsv, bool clearStencil) const noexcept
{
	float rgba[] = {
		color[0], color[1], color[2], color[3]
	};

	mDeviceContext->ClearRenderTargetView(rtv, rgba);

	UINT clear_flags = D3D11_CLEAR_DEPTH;
	if (clearStencil) clear_flags |= D3D11_CLEAR_STENCIL;

	mDeviceContext->ClearDepthStencilView(dsv, clear_flags, 1.0f, 0);
}

void DX::DX11::DeviceContext::SetRtvAndDsv(ID3D11RenderTargetView* rtv, ID3D11DepthStencilView* dsv)
{
	ID3D11RenderTargetView* rtvs[] = {
		rtv
	};

	mDeviceContext->OMSetRenderTargets(_countof(rtvs), rtvs, dsv);
}

void DX::DX11::DeviceContext::SetViewport(int width, int height)
{
	auto vp = D3D11_VIEWPORT();
	vp.Width = static_cast<float>(width);
	vp.Height = static_cast<float>(height);
	vp.MaxDepth = 1.0f;
	vp.MinDepth = 0.0f;

	mDeviceContext->RSSetViewports(1u, &vp);
}

void DX::DX11::DeviceContext::SetVertexBuffer(ID3D11Buffer* buffer, ID3D11InputLayout* inputLayout)
{
	using DX11Vertex = DX::Vertex<D3D11_INPUT_ELEMENT_DESC, D3D11_INPUT_CLASSIFICATION>;

	ID3D11Buffer* buffers[] = { buffer };
	UINT stride = sizeof(DX11Vertex);
	auto offset = 0u;

	mDeviceContext->IASetVertexBuffers(0, static_cast<UINT>(_countof(buffers)), buffers, &stride, &offset);
	mDeviceContext->IASetInputLayout(inputLayout);
}

void DX::DX11::DeviceContext::SetIndexBuffer(ID3D11Buffer* buffer)
{
	mDeviceContext->IASetIndexBuffer(buffer, DXGI_FORMAT_R32_UINT, 0u);
}

void DX::DX11::DeviceContext::SetDepthStencilState(ID3D11DepthStencilState* dss)
{
	mDeviceContext->OMSetDepthStencilState(dss, 0u);
}

void DX::DX11::DeviceContext::SetRasterizerState(ID3D11RasterizerState* rasterizerState)
{
	mDeviceContext->RSSetState(rasterizerState);
}

void DX::DX11::DeviceContext::DrawTriangle(UINT indexCount) const noexcept
{
	mDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	mDeviceContext->DrawIndexed(indexCount, 0, 0);
}
