#include "DescriptorHeap.h"
#include "DXHelper.h"
#include "Resource.h"

DX::DX12::DescriptorHeap::DescriptorHeap(ID3D12Device* device, IDXGISwapChain4* swapChain)
{
	CreateRtvDescriptorHeap(device, swapChain);
	CreateDsvDescriptorHeap(device, swapChain);
}

DX::DX12::DescriptorHeap::~DescriptorHeap()
{
	mDepthStencilView.reset();

	for (auto& buffer : mRenderTargets)
		buffer.Reset();

	mDsvDescriptorHeap.Reset();
	mRtvDescriptorHeap.Reset();
}

void DX::DX12::DescriptorHeap::Clear(ID3D12GraphicsCommandList* commandList, const std::array<float, 4>& color, INT currentIndex, bool clearStencil)
{
	auto rtv_handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(mRtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), currentIndex, mRtvIncrementSize);

	FLOAT rgba[] = {
		color[0], color[1], color[2], color[3]
	};

	auto barrier_1 = CD3DX12_RESOURCE_BARRIER::Transition(mRenderTargets[currentIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);

	commandList->ResourceBarrier(1u, &barrier_1);
	commandList->ClearRenderTargetView(rtv_handle, rgba, 0u, nullptr);

	auto dsv_handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(mDsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

	auto barrier_2 = CD3DX12_RESOURCE_BARRIER::Transition(mDepthStencilView->GetResource(), D3D12_RESOURCE_STATE_DEPTH_READ, D3D12_RESOURCE_STATE_DEPTH_WRITE);

	commandList->ResourceBarrier(1u, &barrier_2);

	D3D12_CLEAR_FLAGS clear_flags = D3D12_CLEAR_FLAG_DEPTH;
	if (clearStencil) clear_flags |= D3D12_CLEAR_FLAG_STENCIL;

	commandList->ClearDepthStencilView(dsv_handle, clear_flags, 1.0f, 0, 0u, nullptr);
}

void DX::DX12::DescriptorHeap::PrepareForPresent(ID3D12GraphicsCommandList* commandList, INT currentIndex)
{
	auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(mRenderTargets[currentIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	commandList->ResourceBarrier(1u, &barrier);

	auto barrier_2 = CD3DX12_RESOURCE_BARRIER::Transition(mDepthStencilView->GetResource(), D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_DEPTH_READ);

	commandList->ResourceBarrier(1u, &barrier_2);
}

void DX::DX12::DescriptorHeap::CreateRtvDescriptorHeap(ID3D12Device* device, IDXGISwapChain4* swapChain)
{
	auto desc = D3D12_DESCRIPTOR_HEAP_DESC();
	desc.NumDescriptors = static_cast<UINT>(DX::BUFFER_COUNT);
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;

	auto res = device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&mRtvDescriptorHeap));
	ThrowIfFailed(res, "Failed to create RTV descriptor heap.\n");

	mRtvIncrementSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	
	auto handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(mRtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
	mRenderTargets.resize(DX::BUFFER_COUNT);

	for (UINT i = 0; i < DX::BUFFER_COUNT; ++i)
	{
		res = swapChain->GetBuffer(i, IID_PPV_ARGS(&mRenderTargets[i]));
		ThrowIfFailed(res, "Failed to get back buffer.\n");
		device->CreateRenderTargetView(mRenderTargets[i].Get(), nullptr, handle);
		handle.Offset(static_cast<INT>(mRtvIncrementSize));
	}
}

void DX::DX12::DescriptorHeap::CreateDsvDescriptorHeap(ID3D12Device* device, IDXGISwapChain4* swapChain)
{
	auto desc = D3D12_DESCRIPTOR_HEAP_DESC();
	desc.NumDescriptors = 1u;
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;

	auto res = device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&mDsvDescriptorHeap));
	ThrowIfFailed(res, "Failed to create DSV descriptor heap.\n");

	mDsvIncrementSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

	auto swap_chain_desc = DXGI_SWAP_CHAIN_DESC1();
	res = swapChain->GetDesc1(&swap_chain_desc);

	mDepthStencilView = std::make_unique<Resource>(device, D3D12_HEAP_TYPE_DEFAULT, D3D12_HEAP_FLAG_NONE, DX::ResourceType::Texture2D, D3D12_RESOURCE_STATE_DEPTH_READ, static_cast<UINT64>(swap_chain_desc.Width), swap_chain_desc.Height, DXGI_FORMAT_D32_FLOAT, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL, D3D12_TEXTURE_LAYOUT_UNKNOWN, 0ull, &(CD3DX12_CLEAR_VALUE(DXGI_FORMAT_D32_FLOAT, 1.0f, 0)));

	device->CreateDepthStencilView(mDepthStencilView->GetResource(), nullptr, mDsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
}
