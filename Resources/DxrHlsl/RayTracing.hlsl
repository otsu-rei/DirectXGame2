RaytracingAccelerationStructure gRtScene : register(t0);
RWTexture2D<float4> gOutput : register(u0);

struct Payload {
	float3 color;
	uint recursive;
};
struct MyAttrbute {
	float2 barys;
};

float3 CalcBarycentrics(float2 barys) {
	return float3(
        1.0f - barys.x - barys.y,
		barys.x,
		barys.y);
}

struct Vertex {
	float3 Position;
	float3 Normal;
	float4 Color;
};

StructuredBuffer<uint> indexBuffer : register(t0, space1);
StructuredBuffer<Vertex> vertexBuffer : register(t1, space1);

// return to color
float3 Reflection(float3 vertexPos, float3 vertexNormal, float3 vertexColor, int recursive) {
	float3 worldPos = mul(float4(vertexPos, 1), ObjectToWorld4x3());
	float3 worldNormal = mul(vertexNormal, (float3x3)ObjectToWorld4x3());
	float3 rayDir = WorldRayDirection();
	
	float3 reflectDir = reflect(rayDir, worldNormal);
	
	RayDesc rayDesc;
	rayDesc.Origin = worldPos;
	rayDesc.Direction = reflectDir;
	rayDesc.TMin = 0.01f;
	rayDesc.TMax = 10000.0f;
	
	Payload reflectPayload;
	reflectPayload.color = float3(0.0f, 0.0f, 0.0f);
	reflectPayload.recursive = recursive;
	
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
		reflectPayload
	);
	
	float3 result = reflectPayload.color * 0.8f + vertexColor * 0.2f;
	
	return result;
	
};

Vertex GetHitVertex(MyAttrbute attrib) {
	Vertex v = (Vertex)0;
	float3 barycentrics = CalcBarycentrics(attrib.barys);
	uint vertexId = PrimitiveIndex() * 3; // Triangle List のため.

	float weights[3] = {
		barycentrics.x, barycentrics.y, barycentrics.z
	};
	for (int i = 0; i < 3; ++i) {
		uint index = indexBuffer[vertexId + i];
		float w = weights[i];
		v.Position += vertexBuffer[index].Position * w;
		v.Normal += vertexBuffer[index].Normal * w;
		v.Color += vertexBuffer[index].Color * w;
	}
	v.Normal = normalize(v.Normal);
	return v;
}

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
	payload.color = float3(0.0f, 0.0f, 0.0f);
	payload.recursive = 0;
	
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
	payload.recursive++;
	if (payload.recursive > 3) {
		return;
	}
	
	Vertex vtx = GetHitVertex(attrib);
	uint id = InstanceID();
	
	if (id == 0) {
		payload.color = vtx.Color.rgb;
	}
	if (id == 1) {
		payload.color = Reflection(vtx.Position, vtx.Normal, vtx.Color.rgb, payload.recursive);
		return;
	}
}