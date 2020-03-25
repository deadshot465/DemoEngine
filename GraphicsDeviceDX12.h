#pragma once
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <d3d12.h>
#include <dxgi1_6.h>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include <Windows.h>
#include <wrl/client.h>
#include "DXHelper.h"

namespace DX
{
	namespace DX12
	{
		inline const std::vector<Vertex<D3D12_INPUT_ELEMENT_DESC, D3D12_INPUT_CLASSIFICATION>> CUBE_VERTICES = {
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

		class CommandQueue;
		class DescriptorHeap;
		class Pipeline;
		class Resource;
		class SwapChain;

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
			
			ID3D12GraphicsCommandList5* GetCommandList();
			void ResetIntermediateBuffer(UINT64 size);

			Microsoft::WRL::ComPtr<ID3D12Debug3> mDebug = nullptr;
			Microsoft::WRL::ComPtr<IDXGIFactory7> mFactory = nullptr;
			Microsoft::WRL::ComPtr<IDXGIAdapter4> mAdapter = nullptr;
			Microsoft::WRL::ComPtr<ID3D12Device6> mDevice = nullptr;
			Microsoft::WRL::ComPtr<ID3D12InfoQueue> mInfoQueue = nullptr;
			Microsoft::WRL::ComPtr<ID3DBlob> mVertexShaderBlob = nullptr;
			Microsoft::WRL::ComPtr<ID3DBlob> mPixelShaderBlob = nullptr;

			std::unique_ptr<CommandQueue> mCommandQueue = nullptr;
			std::unique_ptr<SwapChain> mSwapChain = nullptr;
			std::unique_ptr<DescriptorHeap> mDescriptorHeap = nullptr;
			std::unique_ptr<Resource> mVertexBuffer = nullptr;
			std::unique_ptr<Resource> mIntermediateBuffer = nullptr;
			std::unique_ptr<Resource> mIndexBuffer = nullptr;
			std::unique_ptr<Pipeline> mPipeline = nullptr;

			D3D12_VERTEX_BUFFER_VIEW mVertexBufferView = {};
			D3D12_INDEX_BUFFER_VIEW mIndexBufferView = {};

			DX::MVP mMvp = {};
			std::unordered_map<std::string, std::unique_ptr<Resource>> mRootDescriptors
				= std::unordered_map<std::string, std::unique_ptr<Resource>>();

			int32_t mWidth = 0;
			int32_t mHeight = 0;
			HWND mHandle = nullptr;
			UINT mCurrentBufferIndex = 0u;
		};
	}
}

