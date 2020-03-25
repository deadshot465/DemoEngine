#pragma once
#include <d3d12.h>
#include <wrl/client.h>
#include "DXHelper.h"

namespace DX
{
	namespace DX12
	{
		class Resource
		{
		public:
			Resource(ID3D12Device* device, D3D12_HEAP_TYPE heapType, D3D12_HEAP_FLAGS heapFlags, DX::ResourceType resourceType, D3D12_RESOURCE_STATES resourceState, UINT64 width, UINT height = 0u, DXGI_FORMAT format = DXGI_FORMAT_B8G8R8A8_UNORM, D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE, D3D12_TEXTURE_LAYOUT layout = D3D12_TEXTURE_LAYOUT_UNKNOWN, UINT64 alignment = 0ull, D3D12_CLEAR_VALUE* clearValues = nullptr);
			~Resource();

			void UpdateStaticResource(ID3D12GraphicsCommandList* commandList, ID3D12Resource* intermediate, const void* data, LONG_PTR rowPitch);
			void UpdateDynamicResource(const void* data, size_t size);

			void TransitionState(D3D12_RESOURCE_STATES oldState, D3D12_RESOURCE_STATES newState, ID3D12GraphicsCommandList* commandList);

			ID3D12Resource1* GetResource() const noexcept { return mResource.Get(); }

		private:
			Microsoft::WRL::ComPtr<ID3D12Resource1> mResource = nullptr;
			void* mMappedResource = nullptr;
		};
	}
}

