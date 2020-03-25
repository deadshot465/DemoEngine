struct PS_INPUT
{
	float4 Position : SV_POSITION;
	float3 Normal : NORMAL;
	float2 TexCoord : TEXCOORD;
};

struct MVP
{
	row_major float4x4 Model;
	row_major float4x4 View;
	row_major float4x4 Projection;
};

ConstantBuffer<MVP> MVPConstant : register(b0);

float4 main(PS_INPUT input) : SV_TARGET
{
	return float4(input.Normal, 1.0);
}