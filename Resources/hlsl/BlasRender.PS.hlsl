//-----------------------------------------------------------------------------------------
// include
//-----------------------------------------------------------------------------------------
#include "BlasRender.hlsli"
#include "Lighting.hlsli"
#include "ToonShading.hlsli"

////////////////////////////////////////////////////////////////////////////////////////////
// PSOutput structure
////////////////////////////////////////////////////////////////////////////////////////////
struct PSOutput {
	float4 color : SV_TARGET0;
};

//=========================================================================================
// Buffers
//=========================================================================================
struct Material {
	float4 color;
};
ConstantBuffer<Material> gMaterial : register(b0);

struct DirectionalLigth {
	float4 color;
	float3 direction;
	float intensity;
};
ConstantBuffer<DirectionalLigth> gLight : register(b1);

////////////////////////////////////////////////////////////////////////////////////////////
// main
////////////////////////////////////////////////////////////////////////////////////////////
PSOutput main(VSOutput input) {
	
	PSOutput output;
	
	output.color = gMaterial.color;
	output.color.rgb *= HalfLambertReflection(input.normal, -gLight.direction);
	output.color.rgb = ToonShading(output.color.rgb, 10);
	
	return output;
	
}
