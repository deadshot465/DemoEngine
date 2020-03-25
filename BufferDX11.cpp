#include "BufferDX11.h"
#include "DXHelper.h"

DX::DX11::Buffer::Buffer(ID3D11Device* device, UINT bindFlag, UINT byteWidth, UINT cpuAccessFlag, D3D11_USAGE usage, const void* initialData, const void* shaderByteCode, SIZE_T byteCodeLength)
{
	auto desc = D3D11_BUFFER_DESC();
	desc.BindFlags = bindFlag;
	desc.ByteWidth = byteWidth;
	desc.CPUAccessFlags = cpuAccessFlag;
	desc.MiscFlags = NULL;
	desc.StructureByteStride = 0;
	desc.Usage = usage;

	auto initial_data = D3D11_SUBRESOURCE_DATA();
	initial_data.pSysMem = initialData;

	auto res = device->CreateBuffer(&desc, initialData ? &initial_data : nullptr, mBuffer.ReleaseAndGetAddressOf());
	ThrowIfFailed(res, "Failed to create buffer.\n");

	if (shaderByteCode)
	{
		auto input_element_desc = Vertex<D3D11_INPUT_ELEMENT_DESC, D3D11_INPUT_CLASSIFICATION>::GetInputElementDesc({ 0, 0, 0 }, { 0, 0, 0 }, D3D11_INPUT_PER_VERTEX_DATA);
		res = device->CreateInputLayout(input_element_desc.data(), static_cast<UINT>(input_element_desc.size()), shaderByteCode, byteCodeLength, mInputLayout.ReleaseAndGetAddressOf());
		ThrowIfFailed(res, "Failed to create input element layout.\n");
	}
}

DX::DX11::Buffer::~Buffer()
{
	if (mInputLayout) mInputLayout.Reset();
}
