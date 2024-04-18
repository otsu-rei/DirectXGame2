#include "Triangle.h"

#include "DxrObjectMethods.h"
#include "DxrCommand.h"
#include "MyEngine.h"

using namespace DxrObject;
using namespace DxrObjectMethod;

void Triangle::Init() {
	// vertex & index の生成
	vertex_ = std::make_unique<BufferResource<Vector3f>>(MyDxrEngine::GetDevicesObj(), 3);

	vertex_->operator[](0) = { 0.0f, 1.0f, 0.0f };
	vertex_->operator[](1) = { 1.0f, -1.0f, 0.0f };
	vertex_->operator[](2) = { -1.0f, -1.0f, 0.0f };

	auto device = MyDxrEngine::GetDevicesObj()->GetDevice();
	auto commandList = MyDxrEngine::GetCommandObj()->GetCommandList();



	D3D12_RAYTRACING_GEOMETRY_DESC geomDesc = {};
	geomDesc.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
	geomDesc.Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE;
	geomDesc.Triangles.VertexBuffer.StartAddress = vertex_->GetGPUVirtualAddress();
	geomDesc.Triangles.VertexBuffer.StrideInBytes = sizeof(Vector3f);
	geomDesc.Triangles.VertexFormat = DXGI_FORMAT_R32G32B32_FLOAT;
	geomDesc.Triangles.VertexCount = vertex_->GetSize();

	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC buildASDesc = {};
	auto& inputs = buildASDesc.Inputs;
	inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
	inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
	inputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_NONE;
	inputs.NumDescs = 1;
	inputs.pGeometryDescs = &geomDesc;

	D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO blasPreBuild = {};
	device->GetRaytracingAccelerationStructurePrebuildInfo(&inputs, &blasPreBuild);

	ComPtr<ID3D12Resource> blasScratch;
	blasScratch = CreateBufferResource(
		device,
		blasPreBuild.ResultDataMaxSizeInBytes,
		D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
		kDefaultHeapProps
	);

	bottomLevelAS_ = CreateBufferResource(
		device,
		blasPreBuild.ResultDataMaxSizeInBytes,
		D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
		D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE,
		kDefaultHeapProps
	);

	buildASDesc.ScratchAccelerationStructureData = blasScratch->GetGPUVirtualAddress();
	buildASDesc.DestAccelerationStructureData = bottomLevelAS_->GetGPUVirtualAddress();

	commandList->BuildRaytracingAccelerationStructure(&buildASDesc, 0, nullptr);

	D3D12_RESOURCE_BARRIER barrier
		= CD3DX12_RESOURCE_BARRIER::UAV(bottomLevelAS_.Get());
	commandList->ResourceBarrier(1, &barrier);

	ComPtr<ID3D12Resource> instanceDescBuffer;
	instanceDescBuffer = CreateBufferResource(
		device,
		sizeof(D3D12_RAYTRACING_INSTANCE_DESC),
		D3D12_RESOURCE_FLAG_NONE,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		kUploadHeapProps
	);

	D3D12_RAYTRACING_INSTANCE_DESC* instanceDesc = nullptr;
	instanceDescBuffer->Map(0, nullptr, (void**)&instanceDesc);

	instanceDesc->InstanceID = 0;
	instanceDesc->InstanceContributionToHitGroupIndex = 0;
	instanceDesc->Flags = D3D12_RAYTRACING_INSTANCE_FLAG_NONE;
	instanceDesc->InstanceMask = 0xFF;
	Matrix4x4 mat = Matrix4x4::MakeIdentity();
	mat = Matrix::Transpose(mat);
	memcpy(instanceDesc->Transform, &mat, sizeof(instanceDesc->Transform));
	instanceDesc->AccelerationStructure = bottomLevelAS_->GetGPUVirtualAddress();

	instanceDescBuffer->Unmap(0, nullptr);

	// tlas
	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS inputTL = {};
	inputTL.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;
	inputTL.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
	inputTL.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_NONE;
	inputTL.NumDescs = 1;

	D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO tlasPreBuild = {};
	device->GetRaytracingAccelerationStructurePrebuildInfo(
		&inputTL, &tlasPreBuild
	);

	ComPtr<ID3D12Resource> tlasScratch;
	tlasScratch = CreateBufferResource(
		device,
		tlasPreBuild.ScratchDataSizeInBytes,
		D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
		kDefaultHeapProps
	);

	topLevelAS_ = CreateBufferResource(
		device,
		tlasPreBuild.ResultDataMaxSizeInBytes,
		D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
		D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE,
		kDefaultHeapProps
	);

	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC tlasDesc = {};
	tlasDesc.Inputs = inputTL;
	tlasDesc.Inputs.InstanceDescs = instanceDescBuffer->GetGPUVirtualAddress();
	tlasDesc.DestAccelerationStructureData = topLevelAS_->GetGPUVirtualAddress();
	tlasDesc.ScratchAccelerationStructureData = tlasScratch->GetGPUVirtualAddress();

	commandList->BuildRaytracingAccelerationStructure(&tlasDesc, 0, nullptr);

	barrier = CD3DX12_RESOURCE_BARRIER::UAV(topLevelAS_.Get());
	commandList->ResourceBarrier(1, &barrier);



}

void Triangle::Term() {
	vertex_.reset();
}
