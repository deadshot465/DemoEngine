#include "GraphicsDeviceDX12.h"
#include <d3dcompiler.h>
#include <stdexcept>
#include "CommandQueue.h"
#include "d3dx12.h"
#include "DescriptorHeap.h"
#include "PipelineDX12.h"
#include "Resource.h"
#include "SwapChainDX12.h"

using namespace Microsoft::WRL;
using DX12Vertex = DX::Vertex<D3D12_INPUT_ELEMENT_DESC, D3D12_INPUT_CLASSIFICATION>;

DX::DX12::GraphicsDevice::GraphicsDevice(HWND hwnd, int32_t width, int32_t height)
	: mHandle(hwnd), mWidth(width), mHeight(height)
{
	try
	{
		auto res = D3D12GetDebugInterface(IID_PPV_ARGS(&mDebug));
		ThrowIfFailed(res, "Failed to get debug interface.\n");
		mDebug->EnableDebugLayer();

		GetAdapter();
		CreateDeviceAndSwapChain(hwnd, width, height);
		CreateInfoQueue();
		LoadShader();
		CreateBuffers();
		
		mPipeline = std::make_unique<Pipeline>(mDevice.Get(), mVertexShaderBlob.Get(), mPixelShaderBlob.Get());
	}
	catch (const std::exception&)
	{
		throw;
	}
}

DX::DX12::GraphicsDevice::~GraphicsDevice()
{
	for (auto& [name, buffer] : mRootDescriptors)
		buffer.reset();

	mPipeline.reset();
	if (mIntermediateBuffer) mIntermediateBuffer.reset();
	mIndexBuffer.reset();
	mVertexBuffer.reset();
	mPixelShaderBlob.Reset();
	mVertexShaderBlob.Reset();
	mInfoQueue.Reset();
	mDescriptorHeap.reset();
	mSwapChain.reset();
	mCommandQueue.reset();
	mDevice.Reset();
	mAdapter.Reset();
	mFactory.Reset();
	mDebug.Reset();
}

void DX::DX12::GraphicsDevice::Render(float deltaTime)
{
	auto command_list = GetCommandList();
	auto rtv_heap = mDescriptorHeap->GetRtvDescriptorHeap();
	auto dsv_heap = mDescriptorHeap->GetDsvDescriptorHeap();

	mDescriptorHeap->Clear(command_list, { 0.0f, 1.0f, 1.0f, 1.0f }, static_cast<INT>(mCurrentBufferIndex));
	command_list->SetGraphicsRootSignature(mPipeline->GetRootSignature());
	command_list->OMSetRenderTargets(1u, &(mDescriptorHeap->GetRtvHandle(mCurrentBufferIndex)), FALSE, &(mDescriptorHeap->GetDsvHandle()));
	
	auto vp = CD3DX12_VIEWPORT(0.0f, 0.0f, static_cast<FLOAT>(mWidth), static_cast<FLOAT>(mHeight));
	auto scissor = CD3DX12_RECT(0l, 0l, std::numeric_limits<LONG>::max(), std::numeric_limits<LONG>::max());

	command_list->RSSetViewports(1u, &vp);
	command_list->RSSetScissorRects(1u, &scissor);
	command_list->SetGraphicsRootConstantBufferView(0u, mRootDescriptors["MVP"]->GetResource()->GetGPUVirtualAddress());
	command_list->IASetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	command_list->SetPipelineState(mPipeline->GetPipelineState());
	command_list->IASetVertexBuffers(0u, 1u, &mVertexBufferView);
	command_list->IASetIndexBuffer(&mIndexBufferView);
	command_list->DrawIndexedInstanced(static_cast<UINT>(CUBE_INDICES.size()), 1u, 0u, 0u, 0u);
	mDescriptorHeap->PrepareForPresent(command_list, static_cast<INT>(mCurrentBufferIndex));
	mCommandQueue->ExecuteCommandList(command_list);
	mSwapChain->Present(false);
}

void DX::DX12::GraphicsDevice::GetAdapter()
{
	auto res = CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(&mFactory));
	ThrowIfFailed(res, "Failed to create DXGI factory.\n");

	ComPtr<IDXGIAdapter1> adapter = nullptr;
	SIZE_T dedicated_memory = 0;

	for (UINT i = 0; mFactory->EnumAdapters1(i, adapter.ReleaseAndGetAddressOf()) != DXGI_ERROR_NOT_FOUND; ++i)
	{
		auto desc = DXGI_ADAPTER_DESC1();
		res = adapter->GetDesc1(&desc);
		
		if (dedicated_memory < desc.DedicatedVideoMemory &&
			(desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) == 0 &&
			SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(&mDevice))))
		{
			dedicated_memory = desc.DedicatedVideoMemory;
			res = adapter.As(&mAdapter);
			ThrowIfFailed(res, "Failed to query DXGI adapter.\n");
		}

		adapter.Reset();
	}
}

void DX::DX12::GraphicsDevice::CreateDeviceAndSwapChain(HWND hwnd, int32_t width, int32_t height)
{
	auto res = D3D12CreateDevice(mAdapter.Get(), D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(&mDevice));
	ThrowIfFailed(res, "Failed to create d3d12 device.\n");
	
	mCommandQueue = std::make_unique<CommandQueue>(mDevice.Get());
	
	BOOL tearing_support = FALSE;
	res = mFactory->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &tearing_support, static_cast<UINT>(sizeof(BOOL)));

	mSwapChain = std::make_unique<SwapChain>(mFactory.Get(), mCommandQueue->GetCommandQueue(), hwnd, width, height, tearing_support);

	mDescriptorHeap = std::make_unique<DescriptorHeap>(mDevice.Get(), mSwapChain->GetSwapChain());
}

void DX::DX12::GraphicsDevice::CreateInfoQueue()
{
	auto res = mDevice.As(&mInfoQueue);
	ThrowIfFailed(res, "Failed to query info queue.\n");

	mInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE);
	mInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);
	mInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, TRUE);

	D3D12_MESSAGE_SEVERITY severities[] = {
		D3D12_MESSAGE_SEVERITY_INFO
	};

	D3D12_MESSAGE_ID ids[] = {
		D3D12_MESSAGE_ID_MAP_INVALID_NULLRANGE,
		D3D12_MESSAGE_ID_UNMAP_INVALID_NULLRANGE,
		D3D12_MESSAGE_ID_CLEARRENDERTARGETVIEW_MISMATCHINGCLEARVALUE
	};

	auto filter = D3D12_INFO_QUEUE_FILTER();
	filter.DenyList.NumSeverities = static_cast<UINT>(_countof(severities));
	filter.DenyList.pSeverityList = severities;
	filter.DenyList.NumIDs = static_cast<UINT>(_countof(ids));
	filter.DenyList.pIDList = ids;

	res = mInfoQueue->PushStorageFilter(&filter);
	ThrowIfFailed(res, "Failed to push storage filter.\n");
}

void DX::DX12::GraphicsDevice::LoadShader()
{
	auto res = D3DReadFileToBlob(L"shader/DX12/VertexShaderDX12.cso", mVertexShaderBlob.ReleaseAndGetAddressOf());
	ThrowIfFailed(res, "Failed to load vertex shader.\n");
	res = D3DReadFileToBlob(L"shader/DX12/PixelShaderDX12.cso", mPixelShaderBlob.ReleaseAndGetAddressOf());
	ThrowIfFailed(res, "Failed to load pixel shader.\n");
}

void DX::DX12::GraphicsDevice::CreateBuffers()
{
	// Vertex Buffer
	UINT64 vertex_buffer_size = CUBE_VERTICES.size() * sizeof(DX12Vertex);
	ResetIntermediateBuffer(vertex_buffer_size);
	mVertexBuffer = std::make_unique<Resource>(mDevice.Get(), D3D12_HEAP_TYPE_DEFAULT, D3D12_HEAP_FLAG_NONE, DX::ResourceType::Buffer, D3D12_RESOURCE_STATE_COPY_DEST, vertex_buffer_size);

	auto command_list = GetCommandList();

	mVertexBuffer->UpdateStaticResource(command_list, mIntermediateBuffer->GetResource(), CUBE_VERTICES.data(), static_cast<LONG_PTR>(vertex_buffer_size));
	mVertexBuffer->TransitionState(D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, command_list);

	mCommandQueue->ExecuteCommandList(command_list);

	mVertexBufferView.BufferLocation = mVertexBuffer->GetResource()->GetGPUVirtualAddress();
	mVertexBufferView.SizeInBytes = static_cast<UINT>(vertex_buffer_size);
	mVertexBufferView.StrideInBytes = static_cast<UINT>(sizeof(DX12Vertex));

	// Index Buffer
	UINT64 index_buffer_size = CUBE_INDICES.size() * sizeof(UINT);
	ResetIntermediateBuffer(index_buffer_size);
	mIndexBuffer = std::make_unique<Resource>(mDevice.Get(), D3D12_HEAP_TYPE_DEFAULT, D3D12_HEAP_FLAG_NONE, DX::ResourceType::Buffer, D3D12_RESOURCE_STATE_COPY_DEST, index_buffer_size);

	command_list = GetCommandList();
	
	mIndexBuffer->UpdateStaticResource(command_list, mIntermediateBuffer->GetResource(), CUBE_INDICES.data(), static_cast<LONG_PTR>(index_buffer_size));
	mIndexBuffer->TransitionState(D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_INDEX_BUFFER, command_list);

	mCommandQueue->ExecuteCommandList(command_list);

	mIndexBufferView.BufferLocation = mIndexBuffer->GetResource()->GetGPUVirtualAddress();
	mIndexBufferView.Format = DXGI_FORMAT_R32_UINT;
	mIndexBufferView.SizeInBytes = index_buffer_size;

	// Root Level CBV
	UINT64 mvp_size = sizeof(DX::MVP);
	ResetIntermediateBuffer(mvp_size);
	mRootDescriptors["MVP"] = std::make_unique<Resource>(mDevice.Get(), D3D12_HEAP_TYPE_DEFAULT, D3D12_HEAP_FLAG_NONE, DX::ResourceType::Buffer, D3D12_RESOURCE_STATE_COPY_DEST, mvp_size);

	command_list = GetCommandList();

	auto model = DirectX::XMLoadFloat4x4A(&mMvp.Model);
	auto view = DirectX::XMLoadFloat4x4A(&mMvp.View);
	auto projection = DirectX::XMLoadFloat4x4A(&mMvp.Projection);

	auto eye = DirectX::XMFLOAT3(-5.0f, 2.5f, -5.0f);
	auto focus = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
	auto up = DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f);

	model = DirectX::XMMatrixIdentity();
	view = DirectX::XMMatrixLookAtLH(DirectX::XMLoadFloat3(&eye), DirectX::XMLoadFloat3(&focus), DirectX::XMLoadFloat3(&up));
	projection = DirectX::XMMatrixPerspectiveFovLH(45.0f, static_cast<float>(mWidth) / static_cast<float>(mHeight), 0.1f, 100.0f);

	DirectX::XMStoreFloat4x4A(&mMvp.Model, model);
	DirectX::XMStoreFloat4x4A(&mMvp.View, view);
	DirectX::XMStoreFloat4x4A(&mMvp.Projection, projection);

	mRootDescriptors["MVP"]->UpdateStaticResource(command_list, mIntermediateBuffer->GetResource(), &mMvp, static_cast<LONG_PTR>(mvp_size));
	mRootDescriptors["MVP"]->TransitionState(D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, command_list);

	mCommandQueue->ExecuteCommandList(command_list);
}

ID3D12GraphicsCommandList5* DX::DX12::GraphicsDevice::GetCommandList()
{
	mCurrentBufferIndex = mSwapChain->GetSwapChain()->GetCurrentBackBufferIndex();
	return mCommandQueue->GetCommandList(mCurrentBufferIndex);
}

void DX::DX12::GraphicsDevice::ResetIntermediateBuffer(UINT64 size)
{
	mIntermediateBuffer.reset(new Resource(mDevice.Get(), D3D12_HEAP_TYPE_UPLOAD, D3D12_HEAP_FLAG_NONE, DX::ResourceType::Buffer, D3D12_RESOURCE_STATE_GENERIC_READ, size));
}
