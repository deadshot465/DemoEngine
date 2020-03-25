#pragma once
#include <d3d11_4.h>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include <Windows.h>
#include <wrl/client.h>
#include "DXHelper.h"

namespace DX
{
	namespace DX11
	{
		inline const std::vector<Vertex<D3D11_INPUT_ELEMENT_DESC, D3D11_INPUT_CLASSIFICATION>> CUBE_VERTICES = {
			{{-0.5f,  0.5f, -0.5f}, { 1.0f,  0.0f,  0.0f}, {0.0f, 0.0f}},
			{{ 0.5f,  0.5f, -0.5f}, { 0.0f,  1.0f,  0.0f}, {1.0f, 0.0f}},
			{{ 0.5f, -0.5f, -0.5f}, { 0.0f,  0.0f,  1.0f}, {1.0f, 1.0f}},
			{{-0.5f, -0.5f, -0.5f}, { 1.0f,  0.0f,  0.0f}, {0.0f, 1.0f}},
									 	    
			{{-0.5f,  0.5f,  0.5f}, { 1.0f,  0.0f,  0.0f}, {0.0f, 0.0f}},
			{{ 0.5f,  0.5f,  0.5f}, { 0.0f,  1.0f,  0.0f}, {1.0f, 0.0f}},
			{{ 0.5f,  0.5f, -0.5f}, { 0.0f,  0.0f,  1.0f}, {1.0f, 1.0f}},
			{{-0.5f,  0.5f, -0.5f}, { 1.0f,  0.0f,  0.0f}, {0.0f, 1.0f}},
									 	    
			{{-0.5f,  0.5f,  0.5f}, { 1.0f,  0.0f,  0.0f}, {0.0f, 0.0f}},
			{{ 0.5f,  0.5f,  0.5f}, { 0.0f,  1.0f,  0.0f}, {1.0f, 0.0f}},
			{{ 0.5f, -0.5f,  0.5f}, { 0.0f,  0.0f,  1.0f}, {1.0f, 1.0f}},
			{{-0.5f, -0.5f,  0.5f}, { 1.0f,  0.0f,  0.0f}, {0.0f, 1.0f}},
									 	    
			{{-0.5f, -0.5f,  0.5f}, { 1.0f,  0.0f,  0.0f}, {0.0f, 0.0f}},
			{{ 0.5f, -0.5f,  0.5f}, { 0.0f,  1.0f,  0.0f}, {1.0f, 0.0f}},
			{{ 0.5f, -0.5f, -0.5f}, { 0.0f,  0.0f,  1.0f}, {1.0f, 1.0f}},
			{{-0.5f, -0.5f, -0.5f}, { 1.0f,  0.0f,  0.0f}, {0.0f, 1.0f}},
									 	    	   
			{{ 0.5f,  0.5f, -0.5f}, { 1.0f,  0.0f,  0.0f}, {0.0f, 0.0f}},
			{{ 0.5f,  0.5f,  0.5f}, { 0.0f,  1.0f,  0.0f}, {1.0f, 0.0f}},
			{{ 0.5f, -0.5f,  0.5f}, { 0.0f,  0.0f,  1.0f}, {1.0f, 1.0f}},
			{{ 0.5f, -0.5f, -0.5f}, { 1.0f,  0.0f,  0.0f}, {0.0f, 1.0f}},
									 	    	   
			{{-0.5f,  0.5f, -0.5f}, { 1.0f,  0.0f,  0.0f}, {0.0f, 0.0f}},
			{{-0.5f,  0.5f,  0.5f}, { 0.0f,  1.0f,  0.0f}, {1.0f, 0.0f}},
			{{-0.5f, -0.5f,  0.5f}, { 0.0f,  0.0f,  1.0f}, {1.0f, 1.0f}},
			{{-0.5f, -0.5f, -0.5f}, { 1.0f,  0.0f,  0.0f}, {0.0f, 1.0f}},
		};

		inline const std::vector<UINT> CUBE_INDICES =
		{
			0, 1, 2, 2, 3, 0,
			4, 5, 6, 6, 7, 4,

			8, 9, 10, 10, 11, 8,
			12, 13, 14, 14, 15, 12,

			16, 17, 18, 18, 19, 16,
			20, 21, 22, 22, 23, 20
		};

		class Buffer;
		class DeviceContext;
		class SwapChain;

		template <typename T>
		class Shader;

		class GraphicsDevice
		{
		public:
			GraphicsDevice(HWND hwnd, int32_t width, int32_t height);
			~GraphicsDevice();

			void Render(float deltaTime);

		private:
			void GetAdapter();
			void CreateDeviceAndSwapChain(HWND hwnd, int32_t width, int32_t height);
			void CreateInfoQueue();
			void LoadShader();
			void CreateBuffers();
			void CreateRasterizerState();
			void CreateDepthStencilState();

			Microsoft::WRL::ComPtr<IDXGIAdapter3> mAdapter = nullptr;
			Microsoft::WRL::ComPtr<IDXGIFactory5> mFactory = nullptr;
			Microsoft::WRL::ComPtr<ID3D11Device5> mDevice = nullptr;
			Microsoft::WRL::ComPtr<ID3D11InfoQueue> mInfoQueue = nullptr;
			Microsoft::WRL::ComPtr<ID3D11RasterizerState2> mRasterizerState = nullptr;
			Microsoft::WRL::ComPtr<ID3D11DepthStencilState> mDss = nullptr;

			std::unique_ptr<SwapChain> mSwapChain = nullptr;
			std::unique_ptr<DeviceContext> mDeviceContext = nullptr;
			std::unique_ptr<Shader<ID3D11VertexShader>> mVertexShader = nullptr;
			std::unique_ptr<Shader<ID3D11PixelShader>> mPixelShader = nullptr;
			std::unique_ptr<Buffer> mVertexBuffer = nullptr;
			std::unique_ptr<Buffer> mIndexBuffer = nullptr;

			std::unordered_map<std::string, std::unique_ptr<Buffer>> mConstantBuffers
				= std::unordered_map<std::string, std::unique_ptr<Buffer>>();

			DX::MVP mMvp = {};

			HWND mHandle = nullptr;
			int32_t mWidth = 0;
			int32_t mHeight = 0;
			D3D_FEATURE_LEVEL mFeatureLevel = {};
		};
	}
}

