////////////////////////////////////////////////////////////////////////////////////////////
// DefferedOutput structure
////////////////////////////////////////////////////////////////////////////////////////////
struct DefferedOutput {
	float4 albed    : SV_Target0;
	float4 depth    : SV_Target1;
	float4 normal   : SV_Target2;
	float4 worldPos : SV_Target3;
};

////////////////////////////////////////////////////////////////////////////////////////////
// DefferedVSOutput structure
////////////////////////////////////////////////////////////////////////////////////////////
struct DefferedVSOutput {
	float4 position : SV_Position;
	float2 texcoord : TEXCOORD0;
};

////////////////////////////////////////////////////////////////////////////////////////////
// DefferedRendering methods
////////////////////////////////////////////////////////////////////////////////////////////

namespace Deffered {
	
	/* to color */
	float4 ToNormalColor(float3 normal) {
		float4 result;
	
		result.rgb = (normal + 1.0f) * 0.5f;
		result.a   = 1.0f;
	
		return result;
	}
	
	float4 ToDepthColor(float4 position) {
		float4 result;
		
		result.rgb = position.w / 100.0f;
		result.a   = 1.0f;
		
		return result;
	}
	
	float4 ToWorldPosColor(float3 worldPos) {
		float4 result;
		
		result.rgb = worldPos;
		result.a = 1.0f;
		
		return result;
	}
	
	/* to data */
	float4 GetTextureColor(float2 texcoord, Texture2D texture, SamplerState sample) {
		return texture.Sample(sample, texcoord);
	}
	
}