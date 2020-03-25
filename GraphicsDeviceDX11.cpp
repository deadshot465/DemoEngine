#include "GraphicsDeviceDX11.h"
#include "BufferDX11.h"
#include "DeviceContext.h"
#include "ShaderDX11.h"
#include "SwapChainDX11.h"

using namespace Microsoft::WRL;
using DX11Vertex = DX::Vertex<D3D11_INPUT_ELEMENT_DESC, D3D11_INPUT_CLASSIFICATION>;

DX::DX11::GraphicsDevice::GraphicsDevice(HWND hwnd, int32_t width, int32_t height)
	: mHandle(hwnd), mWidth(width), mHeight(height)
{
	try
	{
		GetAdapter();
		CreateDeviceAndSwapChain(hwnd, width, height);
		CreateInfoQueue();
		LoadShader();
		CreateBuffers();
		CreateDepthStencilState();
		CreateRasterizerState();
	}
	catch (const std::exception&)
	{
		throw;
	}
}

DX::DX11::GraphicsDevice::~GraphicsDevice()
{
	for (auto& [name, buffer] : mConstantBuffers)
	{
		buffer.reset();
	}

	mDss.Reset();
	mRasterizerState.Reset();
	mIndexBuffer.reset();
	mVertexBuffer.reset();
	mPixelShader.reset();
	mVertexShader.reset();
	mDeviceContext.reset();
	mSwapChain.reset();
	mInfoQueue.Reset();
	mDevice.Reset();
	mAdapter.Reset();
	mFactory.Reset();
}

void DX::DX11::GraphicsDevice::Render(float deltaTime)
{
	mDeviceContext->Clear({ 1.0f, 1.0f, 0.0f, 1.0f }, mSwapChain->GetRtv(), mSwapChain->GetDsv());
	mDeviceContext->SetRtvAndDsv(mSwapChain->GetRtv(), mSwapChain->GetDsv());
	mDeviceContext->SetViewport(mWidth, mHeight);
	mDeviceContext->SetDepthStencilState(mDss.Get());
	mDeviceContext->SetRasterizerState(mRasterizerState.Get());

	mDeviceContext->SetShader(mVertexShader->GetShader());
	mDeviceContext->SetShader(mPixelShader->GetShader());
	mDeviceContext->SetVertexBuffer(mVertexBuffer->GetBuffer(), mVertexBuffer->GetInputLayout());
	mDeviceContext->SetIndexBuffer(mIndexBuffer->GetBuffer());
	mDeviceContext->SetConstantBuffer(mVertexShader->GetShader(), mConstantBuffers["MVP"]->GetBuffer());
	mDeviceContext->SetConstantBuffer(mPixelShader->GetShader(), mConstantBuffers["MVP"]->GetBuffer());

	mDeviceContext->DrawTriangle(static_cast<UINT>(CUBE_INDICES.size()));

	mSwapChain->Present(false);
}

void DX::DX11::GraphicsDevice::GetAdapter()
{
	HRESULT res = CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(&mFactory));
	ThrowIfFailed(res, "Failed to create DXGI factory.\n");

	auto adapter = ComPtr<IDXGIAdapter1>();
	SIZE_T dedicated_memory = 0;
	D3D_FEATURE_LEVEL feature_levels[] = {
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0
	};

	for (UINT i = 0; mFactory->EnumAdapters1(i, adapter.ReleaseAndGetAddressOf()) != DXGI_ERROR_NOT_FOUND; ++i)
	{
		auto desc = DXGI_ADAPTER_DESC1();
		res = adapter->GetDesc1(&desc);
		ThrowIfFailed(res, "Failed to acquire adapter description.\n");

		if (dedicated_memory < desc.DedicatedVideoMemory &&
			(desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) == 0 &&
			SUCCEEDED(D3D11CreateDevice(adapter.Get(), D3D_DRIVER_TYPE_UNKNOWN, nullptr,
				D3D11_CREATE_DEVICE_DEBUG, feature_levels, _countof(feature_levels),
				D3D11_SDK_VERSION, nullptr, nullptr, nullptr)))
		{
			dedicated_memory = desc.DedicatedVideoMemory;
			res = adapter.As(&mAdapter);
			ThrowIfFailed(res, "Failed to acquire the adapter.\n");
		}
	}

	adapter.Reset();
}

void DX::DX11::GraphicsDevice::CreateDeviceAndSwapChain(HWND hwnd, int32_t width, int32_t height)
{
	D3D_FEATURE_LEVEL feature_levels[] = {
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0
	};

	ID3D11Device* device = nullptr;
	ID3D11DeviceContext* immediate_context = nullptr;
	ComPtr<ID3D11DeviceContext4> device_context = nullptr;

	HRESULT res = D3D11CreateDevice(mAdapter.Get(), D3D_DRIVER_TYPE_UNKNOWN, nullptr, D3D11_CREATE_DEVICE_DEBUG, feature_levels, _countof(feature_levels), D3D11_SDK_VERSION,
		&device, &mFeatureLevel, &immediate_context);
	ThrowIfFailed(res, "Failed to create D3D11 device.\n");

	res = device->QueryInterface(IID_PPV_ARGS(&mDevice));
	ThrowIfFailed(res, "Failed to query D3D11 device.\n");
	res = immediate_context->QueryInterface(IID_PPV_ARGS(&device_context));
	ThrowIfFailed(res, "Failed to query device context.\n");

	BOOL allow_tearing = FALSE;
	mFactory->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &allow_tearing, static_cast<UINT>(sizeof(BOOL)));

	mSwapChain = std::make_unique<SwapChain>(mFactory.Get(), device, hwnd, width, height, allow_tearing);

	mDeviceContext = std::make_unique<DeviceContext>(device_context);

	device->Release();
	immediate_context->Release();
}

void DX::DX11::GraphicsDevice::CreateInfoQueue()
{
	HRESULT res = mDevice.As(&mInfoQueue);
	ThrowIfFailed(res, "Failed to get the info queue.\n");

	mInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_CORRUPTION, TRUE);
	mInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR, TRUE);
	mInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_WARNING, TRUE);

	D3D11_MESSAGE_SEVERITY ignore_severities[] = {
		D3D11_MESSAGE_SEVERITY_INFO
	};

	D3D11_INFO_QUEUE_FILTER filter = {};
	filter.DenyList.NumSeverities = _countof(ignore_severities);
	filter.DenyList.pSeverityList = ignore_severities;

	res = mInfoQueue->PushStorageFilter(&filter);
	ThrowIfFailed(res, "Failed to push info queue filter.\n");
}

void DX::DX11::GraphicsDevice::LoadShader()
{
	mVertexShader = std::make_unique<Shader<ID3D11VertexShader>>(mDevice.Get(), "shader/DX11/VertexShader.cso");
	mPixelShader = std::make_unique<Shader<ID3D11PixelShader>>(mDevice.Get(), "shader/DX11/PixelShader.cso");
}

void DX::DX11::GraphicsDevice::CreateBuffers()
{
	auto vs_raw_data = mVertexShader->GetRawData();

	mVertexBuffer = std::make_unique<Buffer>(mDevice.Get(), D3D11_BIND_VERTEX_BUFFER, static_cast<UINT>(CUBE_VERTICES.size() * sizeof(DX11Vertex)), NULL, D3D11_USAGE_DEFAULT,
		CUBE_VERTICES.data(), vs_raw_data.data(), sizeof(char) * vs_raw_data.size());

	mIndexBuffer = std::make_unique<Buffer>(mDevice.Get(), D3D11_BIND_INDEX_BUFFER, static_cast<UINT>(CUBE_INDICES.size() * sizeof(UINT)), NULL, D3D11_USAGE_DEFAULT, CUBE_INDICES.data());

	auto model = DirectX::XMLoadFloat4x4A(&mMvp.Model);
	auto view = DirectX::XMLoadFloat4x4A(&mMvp.View);
	auto projection = DirectX::XMLoadFloat4x4A(&mMvp.Projection);

	auto eye = DirectX::XMFLOAT3(5.0f, 2.5f, -5.0f);
	auto focus = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
	auto up = DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f);

	model = DirectX::XMMatrixIdentity();
	view = DirectX::XMMatrixLookAtLH(DirectX::XMLoadFloat3(&eye), DirectX::XMLoadFloat3(&focus), DirectX::XMLoadFloat3(&up));
	projection = DirectX::XMMatrixPerspectiveFovLH(45.0f, static_cast<float>(mWidth) / static_cast<float>(mHeight), 0.1f, 100.0f);

	DirectX::XMStoreFloat4x4A(&mMvp.Model, model);
	DirectX::XMStoreFloat4x4A(&mMvp.View, view);
	DirectX::XMStoreFloat4x4A(&mMvp.Projection, projection);

	mConstantBuffers["MVP"] = std::make_unique<Buffer>(mDevice.Get(), D3D11_BIND_CONSTANT_BUFFER, static_cast<UINT>(sizeof(mMvp)), D3D11_CPU_ACCESS_WRITE, D3D11_USAGE_DYNAMIC, &mMvp);
}

void DX::DX11::GraphicsDevice::CreateRasterizerState()
{
	auto desc = D3D11_RASTERIZER_DESC2();
	desc.AntialiasedLineEnable = TRUE;
	desc.ConservativeRaster = D3D11_CONSERVATIVE_RASTERIZATION_MODE_OFF;
	desc.CullMode = D3D11_CULL_NONE;
	desc.DepthBias = 0;
	desc.DepthBiasClamp = 0.0f;
	desc.DepthClipEnable = FALSE;
	desc.FillMode = D3D11_FILL_SOLID;
	desc.ForcedSampleCount = 0u;
	desc.FrontCounterClockwise = FALSE;
	desc.MultisampleEnable = TRUE;
	desc.ScissorEnable = FALSE;
	desc.SlopeScaledDepthBias = 0.0f;

	auto res = mDevice->CreateRasterizerState2(&desc, mRasterizerState.ReleaseAndGetAddressOf());
	ThrowIfFailed(res, "Failed to create rasterizer state.\n");
}

void DX::DX11::GraphicsDevice::CreateDepthStencilState()
{
	auto desc = D3D11_DEPTH_STENCIL_DESC();
	desc.DepthEnable = TRUE;
	desc.DepthFunc = D3D11_COMPARISON_LESS;
	desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	desc.StencilEnable = FALSE;

	auto res = mDevice->CreateDepthStencilState(&desc, mDss.ReleaseAndGetAddressOf());
	ThrowIfFailed(res, "Failed to create depth stencil state.\n");
}
