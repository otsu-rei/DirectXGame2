RaytracingAccelerationStructure gRtScene : register(t0);
RWTexture2D<float4> gOutput : register(u0);

struct Payload {
	float3 color;
};
struct MyAttrbute {
	float2 barys;
};

[shader("raygeneration")]
void mainRayGen() {
	uint3 launchIndex = DispatchRaysIndex();
	float3 launchDim = DispatchRaysDimensions();
	
	float2 crd = float2(launchIndex.xy);
	float2 dims = float2(launchDim.xy);
	
	float2 d = ((crd / dims) * 2.0f - 1.0f);
	float aspectRaito = dims.x / dims.y;
	
	RayDesc rayDesc;
	rayDesc.Origin = float3(0, 0, -2);
	rayDesc.Direction = normalize(float3(d.x * aspectRaito, -d.y, 1));
	rayDesc.TMin = 0.0f;
	rayDesc.TMax = 10000.0f;
	
	Payload payload;
	
	RAY_FLAG flags = RAY_FLAG_NONE;
	uint rayMask = 0xFF;
	TraceRay(
		gRtScene,
		flags,
		rayMask,
		0, // rayIndex
		1,
		0,
		rayDesc,
		payload
	);
	float3 col = payload.color;
	
	gOutput[launchIndex.xy] = float4(col, 1);
	

}

[shader("miss")]
void mainMS(inout Payload payload) {
	payload.color = float3(0.1f, 0.25f, 0.5f);
}

[shader("closesthit")]
void mainCHS(inout Payload payload, in MyAttrbute attrib) {
	float3 col = 0.0f;
	col.x = 1.0f - attrib.barys.x - attrib.barys.y;
	col.y = attrib.barys.x;
	col.z = attrib.barys.y;
	payload.color = col;
}