#include "Fence.h"
#include <chrono>
#include "DXHelper.h"

DX::DX12::Fence::Fence(ID3D12Device* device)
{
	auto res = device->CreateFence(0ull, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&mFence));
	ThrowIfFailed(res, "Failed to create fence.\n");

	mEventHandle = CreateEventW(nullptr, NULL, NULL, nullptr);
}

DX::DX12::Fence::~Fence()
{
	mFence.Reset();
}

UINT64 DX::DX12::Fence::Signal(ID3D12CommandQueue* commandQueue, UINT64& currentFenceValue)
{
	auto fence_value_to_signal = ++currentFenceValue;
	auto res = commandQueue->Signal(mFence.Get(), fence_value_to_signal);
	ThrowIfFailed(res, "Failed to signal the fence.\n");
	return fence_value_to_signal;
}

void DX::DX12::Fence::WaitForFences(ID3D12CommandQueue* commandQueue, UINT64 nextFenceValue)
{
	auto milliseconds = std::chrono::milliseconds::max();

	if (mFence->GetCompletedValue() < nextFenceValue)
	{
		auto res = mFence->SetEventOnCompletion(nextFenceValue, mEventHandle);
		WaitForSingleObject(mEventHandle, static_cast<DWORD>(milliseconds.count()));
	}
}

void DX::DX12::Fence::Flush(ID3D12CommandQueue* commandQueue, UINT64 currentFenceValue)
{
	auto fence_value = Signal(commandQueue, currentFenceValue);
	WaitForFences(commandQueue, fence_value);
}
