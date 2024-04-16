#include "Triangle.h"

#include "DxrObjectMethods.h"
#include "DxrCommand.h"
#include "MyEngine.h"

using namespace DxrObject;

void Triangle::Init() {
	// vertex & index の生成
	vertex_ = std::make_unique<BufferResource<Vector4f>>(MyDxrEngine::GetDevicesObj(), 3);

	vertex_->operator[](0) = { 0.0f, 1.0f, 0.0f, 1.0f };
	vertex_->operator[](1) = { 1.0f, -1.0f, 0.0f, 1.0f };
	vertex_->operator[](2) = { -1.0f, -1.0f, 0.0f, 1.0f };

	auto device = MyDxrEngine::GetDevicesObj()->GetDevice();
	auto commandList = MyDxrEngine::GetCommandObj()->GetCommandList();

	D3D12_RAYTRACING_GEOMETRY_DESC gDesc = {};
	gDesc.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
	gDesc.Triangles.VertexBuffer.StartAddress = vertex_->GetGPUVirtualAddress();
	gDesc.Triangles.VertexBuffer.StrideInBytes = sizeof(Vector4f);
	gDesc.Triangles.VertexFormat = DXGI_FORMAT_R32G32B32A32_FLOAT;
	gDesc.Triangles.VertexCount = vertex_->GetSize();
	gDesc.Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE;

	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS inputBl = {};
	inputBl.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
	inputBl.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_NONE;
	inputBl.NumDescs = 1;
	inputBl.pGeometryDescs = &gDesc;
	inputBl.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;

	D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO infoBl = {};
	device->GetRaytracingAccelerationStructurePrebuildInfo(&inputBl, &infoBl);

	ComPtr<ID3D12Resource> blasScratch;
	blasScratch = DxrObjectMethod::CreateBufferResource(
		MyDxrEngine::GetDevicesObj()->GetDevice(),
		infoBl.ScratchDataSizeInBytes,
		D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
		DxrObjectMethod::kDefaultHeapProps
	);

	bottomLevelAS_ = DxrObjectMethod::CreateBufferResource(
		device,
		infoBl.ResultDataMaxSizeInBytes,
		D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
		D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE,
		DxrObjectMethod::kDefaultHeapProps
	);

	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC blasDesc = {};
	blasDesc.Inputs = inputBl;
	blasDesc.ScratchAccelerationStructureData = blasScratch->GetGPUVirtualAddress();
	blasDesc.DestAccelerationStructureData = bottomLevelAS_->GetGPUVirtualAddress();
	
	commandList->BuildRaytracingAccelerationStructure(&blasDesc, 0, nullptr);

	D3D12_RESOURCE_BARRIER barrier
		= CD3DX12_RESOURCE_BARRIER::UAV(bottomLevelAS_.Get());
	commandList->ResourceBarrier(1, &barrier);

	ComPtr<ID3D12Resource> instanceDescBuffer = nullptr;
	instanceDescBuffer = DxrObjectMethod::CreateBufferResource(
		device,
		sizeof(D3D12_RAYTRACING_INSTANCE_DESC),
		D3D12_RESOURCE_FLAG_NONE,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		DxrObjectMethod::kUploadHeapProps
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
	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS inputTl = {};
	inputTl.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
	inputTl.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_NONE;
	inputTl.NumDescs = 1;
	inputTl.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;

	D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO infoTl = {};
	device->GetRaytracingAccelerationStructurePrebuildInfo(&inputTl, &infoTl);

	ComPtr<ID3D12Resource> tlasScratch;
	tlasScratch = DxrObjectMethod::CreateBufferResource(
		device,
		infoTl.ScratchDataSizeInBytes,
		D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
		DxrObjectMethod::kDefaultHeapProps
	);

	topLevelAS_ = DxrObjectMethod::CreateBufferResource(
		device,
		infoTl.ResultDataMaxSizeInBytes,
		D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
		D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE,
		DxrObjectMethod::kDefaultHeapProps
	);

	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC tlasDesc = {};
	tlasDesc.Inputs = inputTl;
	tlasDesc.Inputs.InstanceDescs = instanceDescBuffer->GetGPUVirtualAddress();
	tlasDesc.DestAccelerationStructureData = topLevelAS_->GetGPUVirtualAddress();
	tlasDesc.ScratchAccelerationStructureData = tlasScratch->GetGPUVirtualAddress();
	commandList->BuildRaytracingAccelerationStructure(&tlasDesc, 0, nullptr);

	barrier = CD3DX12_RESOURCE_BARRIER::UAV(topLevelAS_.Get());
	commandList->ResourceBarrier(1, &barrier);


}