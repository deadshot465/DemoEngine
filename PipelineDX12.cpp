#include "PipelineDX12.h"
#include <algorithm>
#include <iterator>
#include "d3dx12.h"
#include "DXHelper.h"

using namespace Microsoft::WRL;
using DX12Vertex = DX::Vertex<D3D12_INPUT_ELEMENT_DESC, D3D12_INPUT_CLASSIFICATION>;

DX::DX12::Pipeline::Pipeline(ID3D12Device* device, ID3DBlob* vertexShader, ID3DBlob* pixelShader)
{
	CreateRootSignature(device);
	CreatePipelineState(device, vertexShader, pixelShader);
}

DX::DX12::Pipeline::~Pipeline()
{
	mPipelineState.Reset();
	mRootSignature.Reset();
}

void DX::DX12::Pipeline::CreateRootSignature(ID3D12Device* device)
{
	auto feature = D3D12_FEATURE_DATA_ROOT_SIGNATURE();
	feature.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;

	auto res = device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &feature, static_cast<UINT>(sizeof(feature)));
	ThrowIfFailed(res, "Failed to check root signature feature support.\n");

	auto sampler_desc = CD3DX12_STATIC_SAMPLER_DESC(0u);

	CD3DX12_ROOT_PARAMETER1 root_parameters[1];
	root_parameters[0].InitAsConstantBufferView(0u);

	auto versioned_desc = CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC();
	D3D12_ROOT_SIGNATURE_FLAGS root_signature_flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS;
	versioned_desc.Init_1_1(static_cast<UINT>(_countof(root_parameters)), root_parameters, 1u, &sampler_desc, root_signature_flags);

	ID3DBlob* root_signature_blob = nullptr;
	ID3DBlob* error_blob = nullptr;

	res = D3D12SerializeVersionedRootSignature(&versioned_desc, &root_signature_blob, &error_blob);
	if (!root_signature_blob && FAILED(res))
	{
		auto buffer_size = error_blob->GetBufferSize();
		auto error_msg = std::vector<char>(buffer_size);
		memcpy(error_msg.data(), error_blob->GetBufferPointer(), buffer_size);
		ThrowIfFailed(res, error_msg.data());
	}

	res = device->CreateRootSignature(0u, root_signature_blob->GetBufferPointer(), root_signature_blob->GetBufferSize(), IID_PPV_ARGS(&mRootSignature));
	ThrowIfFailed(res, "Failed to create root signature.\n");

	if (root_signature_blob) root_signature_blob->Release();
	if (error_blob) error_blob->Release();
}

void DX::DX12::Pipeline::CreatePipelineState(ID3D12Device* device, ID3DBlob* vertexShader, ID3DBlob* pixelShader)
{
	auto _device = ComPtr<ID3D12Device6>();
	auto res = device->QueryInterface(IID_PPV_ARGS(&_device));
	ThrowIfFailed(res, "Failed to query interface for D3D12 device.\n");

	auto input_layout = D3D12_INPUT_LAYOUT_DESC();
	auto layout_desc = DX12Vertex::GetInputElementDesc({ 0, 0, 0 }, { 0, 0, 0 }, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA);
	input_layout.NumElements = static_cast<UINT>(layout_desc.size());
	input_layout.pInputElementDescs = layout_desc.data();

	auto rasterizer_state = D3D12_RASTERIZER_DESC();
	rasterizer_state.AntialiasedLineEnable = FALSE;
	rasterizer_state.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
	rasterizer_state.CullMode = D3D12_CULL_MODE_NONE;
	rasterizer_state.DepthClipEnable = FALSE;
	rasterizer_state.FillMode = D3D12_FILL_MODE_SOLID;
	rasterizer_state.ForcedSampleCount = 0u;
	rasterizer_state.FrontCounterClockwise = FALSE;
	rasterizer_state.MultisampleEnable = TRUE;
	rasterizer_state.SlopeScaledDepthBias = 0.0f;

	auto rtv_formats = D3D12_RT_FORMAT_ARRAY();
	rtv_formats.NumRenderTargets = static_cast<UINT>(DX::BUFFER_COUNT);
	for (auto i = 0; i < DX::BUFFER_COUNT; ++i)
		rtv_formats.RTFormats[i] = DXGI_FORMAT_B8G8R8A8_UNORM;

	auto sample_desc = DXGI_SAMPLE_DESC();
	sample_desc.Count = 1u;
	sample_desc.Quality = 0u;

	auto stream = CD3DX12_PIPELINE_STATE_STREAM1();
	stream.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	stream.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC1(D3D12_DEFAULT);
	stream.DSVFormat = DXGI_FORMAT_D32_FLOAT;
	stream.InputLayout = input_layout;
	stream.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	stream.pRootSignature = mRootSignature.Get();
	stream.RasterizerState = CD3DX12_RASTERIZER_DESC(rasterizer_state);
	stream.RTVFormats = rtv_formats;
	stream.SampleDesc = sample_desc;
	
	stream.PS = CD3DX12_SHADER_BYTECODE(pixelShader);
	stream.VS = CD3DX12_SHADER_BYTECODE(vertexShader);

	auto stream_desc = D3D12_PIPELINE_STATE_STREAM_DESC();
	stream_desc.pPipelineStateSubobjectStream = &stream;
	stream_desc.SizeInBytes = sizeof(stream);

	res = _device->CreatePipelineState(&stream_desc, IID_PPV_ARGS(&mPipelineState));
	ThrowIfFailed(res, "Failed to create pipeline state.\n");
}