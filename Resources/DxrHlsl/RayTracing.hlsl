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
	uint2 launchIndex = DispatchRaysIndex().xy;
	float2 dims = float2(DispatchRaysDimensions().xy);
	float2 d = (launchIndex.xy + 0.5f) / dims.xy * 2.0f - 1.0f;
	
	RayDesc rayDesc;
	rayDesc.Origin = (d.x, d.y, 1);
	rayDesc.Direction = (0, 0, -1);
	rayDesc.TMin = 0;
	rayDesc.TMax = 100000;
	
	Payload payload;
	payload.color = float3(0, 0, 0);
	
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
	payload.color = float3(0.4f, 0.8f, 0.9f);
}

[shader("closesthit")]
void mainCHS(inout Payload payload, MyAttrbute attrib) {
	float3 col = 0.0f;
	col.xy = attrib.barys;
	col.z = 1.0f - col.x - col.y;
	payload.color = col;
}