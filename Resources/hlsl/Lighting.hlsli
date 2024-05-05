////////////////////////////////////////////////////////////////////////////////////////////
// lambert methods
////////////////////////////////////////////////////////////////////////////////////////////

//! @brief lambert反射の数値を計算
//!
//! @param[in] normal         法線ベクトル
//! @param[in] lightDirection ライトから計算するポリゴンへの方向
//! 
//! @return lambert反射の数値を 0 ~ 1 で返却
float LambertReflection(float3 normal, float3 lightDirection) {
	float result;
	
	result = dot(normalize(normal), -lightDirection);
	
	return saturate(result);
}

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