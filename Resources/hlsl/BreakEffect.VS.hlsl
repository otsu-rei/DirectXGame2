//-----------------------------------------------------------------------------------------
// include
//-----------------------------------------------------------------------------------------
#include "BreakEffect.hlsli"

////////////////////////////////////////////////////////////////////////////////////////////
// VSInput sturucture
////////////////////////////////////////////////////////////////////////////////////////////
struct VSInput {
	float4 position : POSITION0;
	float2 texcoord : TEXCOORD0;
	float3 normal   : NORMAL0;
};

//=========================================================================================
// Buffers
//=========================================================================================
struct TransformationMatrix {
	float4x4 world;
	float4x4 worldInverseTranspose;
};
ConstantBuffer<TransformationMatrix> gMatrix : register(b0);

struct Camera2D {
	float4x4 proj;
};
ConstantBuffer<Camera2D> gCamera2D : register(b1);

////////////////////////////////////////////////////////////////////////////////////////////
// main
////////////////////////////////////////////////////////////////////////////////////////////
VSOutput main(VSInput input) {
	
	VSOutput output;
	
	output.position = mul(input.position, gMatrix.world * gCamera2D.proj);
	output.texcoord = input.texcoord;
	output.normal   = normalize(mul(input.normal, (float3x3)gMatrix.world));
	
	return output;
	
}