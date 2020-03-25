#pragma once
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <d3d12.h>
#include <wrl/client.h>

namespace DX
{
	namespace DX12
	{
		class Fence
		{
		public:
			Fence(ID3D12Device* device);
			~Fence();

			UINT64 Signal(ID3D12CommandQueue* commandQueue, UINT64& currentFenceValue);
			void WaitForFences(ID3D12CommandQueue* commandQueue, UINT64 nextFenceValue);
			void Flush(ID3D12CommandQueue* commandQueue, UINT64 currentFenceValue);

		private:
			Microsoft::WRL::ComPtr<ID3D12Fence1> mFence = nullptr;
			HANDLE mEventHandle = nullptr;
		};
	}
}

