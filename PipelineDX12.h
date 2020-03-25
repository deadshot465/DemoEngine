#pragma once
#include <d3d12.h>
#include <vector>
#include <wrl/client.h>

namespace DX
{
	namespace DX12
	{
		class Pipeline
		{
		public:
			Pipeline(ID3D12Device* device, ID3DBlob* vertexShader, ID3DBlob* pixelShader);
			~Pipeline();

			ID3D12RootSignature* GetRootSignature() const noexcept { return mRootSignature.Get(); }
			ID3D12PipelineState* GetPipelineState() const noexcept { return mPipelineState.Get(); }

		private:
			void CreateRootSignature(ID3D12Device* device);
			void CreatePipelineState(ID3D12Device* device, ID3DBlob* vertexShader, ID3DBlob* pixelShader);

			Microsoft::WRL::ComPtr<ID3D12PipelineState> mPipelineState = nullptr;
			Microsoft::WRL::ComPtr<ID3D12RootSignature> mRootSignature = nullptr;
		};
	}
}

