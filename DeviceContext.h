#pragma once
#include <array>
#include <d3d11_4.h>
#include <wrl/client.h>

namespace DX
{
	namespace DX11
	{
		class DeviceContext
		{
		public:
			DeviceContext(Microsoft::WRL::ComPtr<ID3D11DeviceContext4>& deviceContext);
			~DeviceContext();

			void Clear(const std::array<float, 4>& color, ID3D11RenderTargetView* rtv, ID3D11DepthStencilView* dsv, bool clearStencil = false) const noexcept;
			void SetRtvAndDsv(ID3D11RenderTargetView* rtv, ID3D11DepthStencilView* dsv);
			void SetViewport(int width, int height);
			void SetVertexBuffer(ID3D11Buffer* buffer, ID3D11InputLayout* inputLayout);
			void SetIndexBuffer(ID3D11Buffer* buffer);
			void SetDepthStencilState(ID3D11DepthStencilState* dss);
			void SetRasterizerState(ID3D11RasterizerState* rasterizerState);
			
			template <typename T>
			void SetShader(T* shader);

			template <typename T>
			void SetConstantBuffer(T* shader, ID3D11Buffer* buffer);

			void DrawTriangle(UINT indexCount) const noexcept;

		private:
			Microsoft::WRL::ComPtr<ID3D11DeviceContext4> mDeviceContext = nullptr;
		};
		
		template<typename T>
		inline void DeviceContext::SetShader(T* shader)
		{
			if constexpr (std::is_same_v<T, ID3D11VertexShader>)
				mDeviceContext->VSSetShader(shader, nullptr, 0);
			else if constexpr (std::is_same_v<T, ID3D11PixelShader>)
				mDeviceContext->PSSetShader(shader, nullptr, 0);
		}

		template<typename T>
		inline void DeviceContext::SetConstantBuffer(T* shader, ID3D11Buffer* buffer)
		{
			ID3D11Buffer* buffers[] = { buffer };

			if constexpr (std::is_same_v<T, ID3D11VertexShader>)
				mDeviceContext->VSSetConstantBuffers(0, _countof(buffers), buffers);
			else if constexpr (std::is_same_v<T, ID3D11PixelShader>)
				mDeviceContext->PSSetConstantBuffers(0, _countof(buffers), buffers);
		}

	}
}

