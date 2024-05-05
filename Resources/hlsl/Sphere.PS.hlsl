//-----------------------------------------------------------------------------------------
// include
//-----------------------------------------------------------------------------------------
//#include "Lighting.hlsli"
#include "Sphere.hlsli"

////////////////////////////////////////////////////////////////////////////////////////////
// PSOutput structure
////////////////////////////////////////////////////////////////////////////////////////////
struct PSOutput {
	float4 color : SV_TARGET0;
};

//! @brief lambert反射の数値を計算
//!
//! @param[in] normal         法線ベクトル
//! @param[in] lightDirection ライトから計算するポリゴンへの方向
//! @param[in] exponent       halfLambertの指数
//! 
//! @return lambert反射の数値を 0 ~ 1 で返却
float HalfLambertReflection(float3 normal, float3 lightDirection, float exponent = 2.0f) {
	float result;
	
	float NdotL = dot(normalize(normal), -lightDirection);
	result = pow(NdotL * 0.5f + 0.5f, exponent);
	
	return result;
}

////////////////////////////////////////////////////////////////////////////////////////////
// メイン
////////////////////////////////////////////////////////////////////////////////////////////
PSOutput main(VSOutput input) {
	
	PSOutput output;
	
	const float4 color = float4(1.0f, 1.0f, 1.0f, 1.0f);
	const float3 direction = float3(1.0f, 0.0f, 0.0f);
	
	// textureSampler
	output.color.rgb = HalfLambertReflection(input.normal, direction) * color.rgb;
	output.color.a = color.a;
	
	if (output.color.a == 0.0f) {
		discard;
	}
	
	return output;
}