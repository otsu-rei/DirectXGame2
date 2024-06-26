#include "Particle.hlsli"

//struct TransformationMatrix {
//	float4x4 WVP;
//	float4x4 World;
//};
//StructuredBuffer<TransformationMatrix> gTransformationMatrices : register(t0);

struct ParticleForGPU {
	float4x4 WVP;
	float4x4 World;
	float4   color;
};
StructuredBuffer<ParticleForGPU> gParticle : register(t0);

struct VSInput {
	float4 position : POSITION0;
	float2 texcoord : TEXCOORD0;
	float3 normal   : NORMAL0;
};


VSOutput main(VSInput input, uint instanceId : SV_InstanceID) {
	
	VSOutput output;
	output.position = mul(input.position, gParticle[instanceId].WVP);
	output.worldPos = mul(input.position, gParticle[instanceId].World);
	output.texcoord = input.texcoord;
	output.normal = normalize(mul(input.normal, (float3x3)gParticle[instanceId].World));
	output.color = gParticle[instanceId].color;
	return output;
}