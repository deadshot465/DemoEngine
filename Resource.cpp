#include "Resource.h"
#include "d3dx12.h"
#include <cassert>

DX::DX12::Resource::Resource(ID3D12Device* device, D3D12_HEAP_TYPE heapType, D3D12_HEAP_FLAGS heapFlags, DX::ResourceType resourceType, D3D12_RESOURCE_STATES resourceState, UINT64 width, UINT height, DXGI_FORMAT format, D3D12_RESOURCE_FLAGS flags, D3D12_TEXTURE_LAYOUT layout, UINT64 alignment, D3D12_CLEAR_VALUE* clearValues)
{
	auto properties = CD3DX12_HEAP_PROPERTIES(heapType);
	auto desc = CD3DX12_RESOURCE_DESC();

	switch (resourceType)
	{
	case DX::ResourceType::Buffer:
		desc = CD3DX12_RESOURCE_DESC::Buffer(width, flags, alignment);
		break;
	case DX::ResourceType::Texture2D:
		desc = CD3DX12_RESOURCE_DESC::Tex2D(format, width, height, 1u, 0u, 1u, 0u, flags, layout, alignment);
		break;
	default:
		break;
	}

	auto res = device->CreateCommittedResource(&properties, heapFlags, &desc, resourceState, clearValues, IID_PPV_ARGS(&mResource));
	ThrowIfFailed(res, "Failed to create committed resource.\n");
}

DX::DX12::Resource::~Resource()
{
	if (mMappedResource)
	{
		auto range = CD3DX12_RANGE(0ull, 0ull);
		mResource->Unmap(0u, &range);
		mMappedResource = nullptr;
	}

	mResource.Reset();
}

void DX::DX12::Resource::UpdateStaticResource(ID3D12GraphicsCommandList* commandList, ID3D12Resource* intermediate, const void* data, LONG_PTR rowPitch)
{
	auto subresource_data = D3D12_SUBRESOURCE_DATA();
	subresource_data.pData = data;
	subresource_data.RowPitch = rowPitch;
	subresource_data.SlicePitch = rowPitch;

	UpdateSubresources(commandList, mResource.Get(), intermediate, 0ull, 0u, 1u, &subresource_data);
}

void DX::DX12::Resource::UpdateDynamicResource(const void* data, size_t size)
{
	if (!mMappedResource)
	{
		assert(mResource);

		auto range = CD3DX12_RANGE(0ull, 0ull);
		auto res = mResource->Map(0u, &range, &mMappedResource);
		ThrowIfFailed(res, "Failed to map dynamic subresource.\n");
	}

	memcpy(mMappedResource, data, size);
}

void DX::DX12::Resource::TransitionState(D3D12_RESOURCE_STATES oldState, D3D12_RESOURCE_STATES newState, ID3D12GraphicsCommandList* commandList)
{
	auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(mResource.Get(), oldState, newState);
	commandList->ResourceBarrier(1u, &barrier);
}
