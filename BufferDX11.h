#pragma once
#include "IBufferDX.h"
#include <d3d11_4.h>

namespace DX
{
	namespace DX11
	{
		class Buffer :
			public IBuffer<ID3D11Buffer>
		{
		public:
			Buffer(ID3D11Device* device, UINT bindFlag, UINT byteWidth, UINT cpuAccessFlag, D3D11_USAGE usage, const void* initialData = nullptr, const void* shaderByteCode = nullptr, SIZE_T byteCodeLength = 0);
			~Buffer();

			ID3D11InputLayout* GetInputLayout() const noexcept { return mInputLayout.Get(); }
		private:
			Microsoft::WRL::ComPtr<ID3D11InputLayout> mInputLayout = nullptr;
		};
	}
}

