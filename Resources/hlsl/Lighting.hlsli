////////////////////////////////////////////////////////////////////////////////////////////
// lambert methods
////////////////////////////////////////////////////////////////////////////////////////////

float LambertReflection(float3 normal, float3 toLightDirection) {
	float result;
	
	result = dot(normal, toLightDirection);
	
	return saturate(result);
}

float HalfLambertReflection(float3 normal, float3 toLightDirection, float exponent = 2.0f) {
	float result;
	
	float NdotL = dot(normal, toLightDirection);
	result = pow(NdotL * 0.5f + 0.5f, exponent);
	
	return result;
}

float BlinnPhong(float3 normal, float3 toLightDirection, float3 toViewDirection, float specPow) {
	float result;
	
	float3 halfVector = normalize(toLightDirection + toViewDirection);
	float NdotH = max(0, dot(normal, halfVector));
	result = pow(NdotH, specPow);
	
	
	return result;
}