#pragma once
#include <array>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <memory>
#include <vector>
#include <wrl/client.h>
#include "d3dx12.h"

namespace DX
{
	namespace DX12
	{
		class Resource;

		class DescriptorHeap
		{
		public:
			DescriptorHeap(ID3D12Device* device, IDXGISwapChain4* swapChain);
			~DescriptorHeap();

			void Clear(ID3D12GraphicsCommandList* commandList, const std::array<float, 4>& color, INT currentIndex, bool clearStencil = false);
			void PrepareForPresent(ID3D12GraphicsCommandList* commandList, INT currentIndex);

			ID3D12DescriptorHeap* GetRtvDescriptorHeap() const noexcept { return mRtvDescriptorHeap.Get(); }
			ID3D12DescriptorHeap* GetDsvDescriptorHeap() const noexcept { return mDsvDescriptorHeap.Get(); }
			
			D3D12_CPU_DESCRIPTOR_HANDLE GetRtvHandle(UINT currentIndex) { return CD3DX12_CPU_DESCRIPTOR_HANDLE(mRtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), static_cast<INT>(currentIndex), mRtvIncrementSize); }

			D3D12_CPU_DESCRIPTOR_HANDLE GetDsvHandle() { return CD3DX12_CPU_DESCRIPTOR_HANDLE(mDsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart()); }

		private:
			void CreateRtvDescriptorHeap(ID3D12Device* device, IDXGISwapChain4* swapChain);
			void CreateDsvDescriptorHeap(ID3D12Device* device, IDXGISwapChain4* swapChain);

			Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mRtvDescriptorHeap;
			Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mDsvDescriptorHeap;
			std::vector<Microsoft::WRL::ComPtr<ID3D12Resource1>> mRenderTargets;
			std::unique_ptr<Resource> mDepthStencilView = nullptr;
			
			UINT mRtvIncrementSize = 0u;
			UINT mDsvIncrementSize = 0u;
		};
	}
}

