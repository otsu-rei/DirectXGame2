////////////////////////////////////////////////////////////////////////////////////////////
// VSOutput structure
////////////////////////////////////////////////////////////////////////////////////////////
struct VSOutput {
	float4 position : SV_POSITION;
	float4 worldPos : POSITION0;
	float2 texcoord : TEXCOORD0;
	float3 normal   : NORMAL0;
};

////////////////////////////////////////////////////////////////////////////////////////////
// GSOutput structure
////////////////////////////////////////////////////////////////////////////////////////////
struct GSOutput {
	float4 position : SV_POSITION;
	float4 worldPos : POSITION0;
	float2 texcoord : TEXCOORD0;
	float3 normal   : NORMAL0;
};