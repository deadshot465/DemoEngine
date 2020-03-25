#pragma once
#include <d3d12.h>
#include <memory>
#include <vector>
#include <wrl/client.h>
#include "DXHelper.h"

namespace DX
{
	namespace DX12
	{
		class Fence;

		class CommandQueue
		{
		public:
			CommandQueue(ID3D12Device* device);
			~CommandQueue();

			void ExecuteCommandList(ID3D12GraphicsCommandList* commandList);
			ID3D12GraphicsCommandList5* GetCommandList(UINT index);

			ID3D12CommandQueue* GetCommandQueue() const noexcept { return mCommandQueue.Get(); }

		private:
			std::vector<Microsoft::WRL::ComPtr<ID3D12CommandAllocator>> mCommandAllocators;
			Microsoft::WRL::ComPtr<ID3D12CommandQueue> mCommandQueue = nullptr;
			std::vector<Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList5>> mCommandLists;
			std::unique_ptr<Fence> mFence = nullptr;

			UINT64 mCurrentFenceValue = 0ull;
			UINT64 mFenceValues[DX::BUFFER_COUNT] = { 0ull };
			UINT mCurrentBufferIndex = 0u;
		};
	}
}

