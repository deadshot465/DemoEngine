#include "CommandQueue.h"
#include "Fence.h"

using namespace Microsoft::WRL;

DX::DX12::CommandQueue::CommandQueue(ID3D12Device* device)
{
	mCommandAllocators.resize(2 * DX::THREAD_COUNT + DX::BUNDLE_COUNT);

	auto queue_desc = D3D12_COMMAND_QUEUE_DESC();
	queue_desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
	queue_desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

	auto res = device->CreateCommandQueue(&queue_desc, IID_PPV_ARGS(&mCommandQueue));
	ThrowIfFailed(res, "Failed to create command queue.\n");

	for (auto& allocator : mCommandAllocators)
	{
		res = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&allocator));
		ThrowIfFailed(res, "Failed to create command allocator.\n");
	}

	// Temporarily using only a single command list.
	// TODO: Understand multhreading command list recording.
	mCommandLists.resize(1);
	auto _device = ComPtr<ID3D12Device6>();
	device->QueryInterface(IID_PPV_ARGS(&_device));
	res = _device->CreateCommandList1(0u, D3D12_COMMAND_LIST_TYPE_DIRECT, D3D12_COMMAND_LIST_FLAG_NONE, IID_PPV_ARGS(&mCommandLists[0]));

	mFence = std::make_unique<Fence>(_device.Get());

	_device.Reset();
}

DX::DX12::CommandQueue::~CommandQueue()
{
	mFence->Flush(mCommandQueue.Get(), mCurrentFenceValue);
	mFence.reset();

	for (auto& list : mCommandLists)
		list.Reset();

	for (auto& allocator : mCommandAllocators)
		allocator.Reset();

	mCommandQueue.Reset();
}

void DX::DX12::CommandQueue::ExecuteCommandList(ID3D12GraphicsCommandList* commandList)
{
	auto res = commandList->Close();
	ThrowIfFailed(res, "Failed to close the command list.\n");

	ID3D12CommandList* lists[] = {
		commandList
	};

	mCommandQueue->ExecuteCommandLists(static_cast<UINT>(_countof(lists)), lists);
	mFenceValues[mCurrentBufferIndex] = mFence->Signal(mCommandQueue.Get(), mCurrentFenceValue);
}

ID3D12GraphicsCommandList5* DX::DX12::CommandQueue::GetCommandList(UINT index)
{
	mCurrentBufferIndex = index;
	mFence->WaitForFences(mCommandQueue.Get(), mFenceValues[mCurrentBufferIndex]);

	mCommandAllocators[index]->Reset();
	auto res = mCommandLists[0]->Reset(mCommandAllocators[index].Get(), nullptr);
	ThrowIfFailed(res, "Failed to reset command list.\n");

	return mCommandLists[0].Get();
}
