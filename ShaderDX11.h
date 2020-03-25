#pragma once
#include <d3d11_4.h>
#include <string_view>
#include <type_traits>
#include <wrl/client.h>
#include "Common.h"
#include "DXHelper.h"

namespace DX
{
	namespace DX11
	{
		template <typename T>
		class Shader
		{
		public:
			Shader(ID3D11Device* device, std::string_view fileName);
			virtual ~Shader();

			T* GetShader() const noexcept { return mShader.Get(); }
			const std::vector<char>& GetRawData() const noexcept { return mRawData; }

		protected:
			Microsoft::WRL::ComPtr<T> mShader = nullptr;
			std::vector<char> mRawData;
		};

		template<typename T>
		inline Shader<T>::Shader(ID3D11Device* device, std::string_view fileName)
		{
			auto data = ReadFromFile(fileName);
			HRESULT res = S_OK;

			if constexpr (std::is_same_v<T, ID3D11VertexShader>)
				res = device->CreateVertexShader(data.data(), data.size(), nullptr, mShader.ReleaseAndGetAddressOf());
			else if constexpr (std::is_same_v<T, ID3D11PixelShader>)
				res = device->CreatePixelShader(data.data(), data.size(), nullptr, mShader.ReleaseAndGetAddressOf());

			ThrowIfFailed(res, "Failed to load and compile shader.\n");
			mRawData = std::move(data);
		}

		template<typename T>
		inline Shader<T>::~Shader()
		{
			mShader.Reset();
		}
	}
}

