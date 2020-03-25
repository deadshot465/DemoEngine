#pragma once
#include <array>
#include <DirectXMath.h>
#include <dxgi1_6.h>
#include <stdexcept>
#include <string_view>
#include <vector>
#include <Windows.h>

namespace DX
{
	template <typename T, typename U>
	struct Vertex
	{
	public:
		DirectX::XMFLOAT3 Position;
		DirectX::XMFLOAT3 Normal;
		DirectX::XMFLOAT2 TexCoord;

		static std::array<T, 3> GetInputElementDesc(const std::array<UINT, 3>& inputSlots, const std::array<UINT, 3>& semanticIndex, U inputClass) noexcept
		{
			auto descs = std::array<T, 3>();
			descs[0].AlignedByteOffset = offsetof(Vertex, Position);
			descs[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
			descs[0].InputSlot = inputSlots[0];
			descs[0].InputSlotClass = inputClass;
			descs[0].InstanceDataStepRate = 0;
			descs[0].SemanticIndex = semanticIndex[0];
			descs[0].SemanticName = "POSITION";

			descs[1].AlignedByteOffset = offsetof(Vertex, Normal);
			descs[1].Format = DXGI_FORMAT_R32G32B32_FLOAT;
			descs[1].InputSlot = inputSlots[1];
			descs[1].InputSlotClass = inputClass;
			descs[1].InstanceDataStepRate = 0;
			descs[1].SemanticIndex = semanticIndex[1];
			descs[1].SemanticName = "NORMAL";

			descs[2].AlignedByteOffset = offsetof(Vertex, TexCoord);
			descs[2].Format = DXGI_FORMAT_R32G32_FLOAT;
			descs[2].InputSlot = inputSlots[2];
			descs[2].InputSlotClass = inputClass;
			descs[2].InstanceDataStepRate = 0;
			descs[2].SemanticIndex = semanticIndex[2];
			descs[2].SemanticName = "TEXCOORD";

			return descs;
		}
	};

	struct MVP
	{
	public:
		DirectX::XMFLOAT4X4A Model;
		DirectX::XMFLOAT4X4A View;
		DirectX::XMFLOAT4X4A Projection;
	};

	enum class ResourceType
	{
		Buffer, Texture2D
	};

	inline constexpr int32_t BUFFER_COUNT = 3;
	inline constexpr int32_t THREAD_COUNT = 5;
	inline constexpr int32_t BUNDLE_COUNT = 3;

	inline void ThrowIfFailed(HRESULT res, std::string_view msg)
	{
		if (FAILED(res))
			throw std::runtime_error(msg.data());
	}
}
