//-----------------------------------------------------------------------------------------
// include
//-----------------------------------------------------------------------------------------
#include "BreakEffect.hlsli"
#include "Lighting.hlsli"

////////////////////////////////////////////////////////////////////////////////////////////
// PSOutput structure
////////////////////////////////////////////////////////////////////////////////////////////
struct PSOutput {
	float4 color : SV_TARGET0;
};

//=========================================================================================
// TextureBuffers
//=========================================================================================
Texture2D<float4> gTexture : register(t0);
SamplerState gSampler : register(s0);

////////////////////////////////////////////////////////////////////////////////////////////
// Main
////////////////////////////////////////////////////////////////////////////////////////////
PSOutput main(VSOutput input) {
	
	PSOutput output;
	
	output.color = gTexture.Sample(gSampler, input.texcoord);
	
	output.color.rgb += float3(0.4f, 0.4f, 0.4f) * BlinnPhong(input.normal, float3(0.0f, 1.0f, 0.0f), float3(0.0f, 0.0f, 1.0f), 100.0f);
	output.color = saturate(output.color);
	
	if (output.color.a == 0.0f) { //!< 透明度0の場合はpixel破棄
		discard;
	}
	
	return output;
	
}