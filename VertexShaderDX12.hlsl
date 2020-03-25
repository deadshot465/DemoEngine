struct VS_INPUT
{
	float3 Position : POSITION;
	float3 Normal : NORMAL;
	float2 TexCoord : TEXCOORD;
};

struct VS_OUTPUT
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

VS_OUTPUT main(VS_INPUT input)
{
	VS_OUTPUT output = (VS_OUTPUT)0;
	output.Position = float4(input.Position, 1.0);
	output.Position = mul(output.Position, MVPConstant.Model);
	output.Position = mul(output.Position, MVPConstant.View);
	output.Position = mul(output.Position, MVPConstant.Projection);
	output.Normal = input.Normal;
	output.TexCoord = input.TexCoord;
	return output;
}