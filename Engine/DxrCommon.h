#pragma once

//*****************************************************************************************
// DxrCommonは DirectXRaytracing を使うために作られたもの
// DxrObjectを使用する
// DxCommon - DxObject は使わない
//*****************************************************************************************

//-----------------------------------------------------------------------------------------
// include
//-----------------------------------------------------------------------------------------
// c++
#include <memory>

// DxrObject
#include <DxrDevices.h>
#include <DxrCommand.h>
#include <DxrDescriptorHeaps.h>
#include <DxrSwapChain.h>
#include <DxrFence.h>

#include <Vector4.h>
#include <Vector3.h>
#include <Vector2.h>
#include <Matrix4x4.h>

// directX
#include <d3d12.h>
#include <dxgi1_6.h>
#include <dxcapi.h>

// c++
#include <cassert>
#include <vector>

#include <ComPtr.h>

//-----------------------------------------------------------------------------------------
// comment
//-----------------------------------------------------------------------------------------
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxcompiler.lib")

////////////////////////////////////////////////////////////////////////////////////////////
// PolygonMesh structure
////////////////////////////////////////////////////////////////////////////////////////////
struct PolygonMesh {
	ComPtr<ID3D12Resource> vertexBuffer;
	ComPtr<ID3D12Resource> indexBuffer;

	DxrObject::Descriptor descriptorVB;
	DxrObject::Descriptor descriptorIB;

	UINT vertexCount = 0;
	UINT indexCount = 0;
	UINT vertexStride = 0;

	ComPtr<ID3D12Resource> blas;

	std::wstring shaderName;
};

////////////////////////////////////////////////////////////////////////////////////////////
// Vertex structure
////////////////////////////////////////////////////////////////////////////////////////////
struct VertexDataDXR {
	Vector3f position;
	Vector3f normal;
	Vector4f color;
};

////////////////////////////////////////////////////////////////////////////////////////////
// AccelerationStructureBuffers structure
////////////////////////////////////////////////////////////////////////////////////////////
struct AccelerationStructureBuffers {
	ComPtr<ID3D12Resource> scratch;
	ComPtr<ID3D12Resource> asbuffer;
	ComPtr<ID3D12Resource> update;
};


//-----------------------------------------------------------------------------------------
// forward
//-----------------------------------------------------------------------------------------
class WinApp;

////////////////////////////////////////////////////////////////////////////////////////////
// DxrCommon class
////////////////////////////////////////////////////////////////////////////////////////////
class DxrCommon {
public:

	//=========================================================================================
	// public methods
	//=========================================================================================

	//! @brief コンストラクタ
	DxrCommon() {}

	//! @brief デストラクタ
	~DxrCommon() {}

	//! @brief 初期化処理
	void Init(WinApp* winApp, int32_t clientWidth, int32_t clientHeight);

	//! @brief 終了処理
	void Term();

	// default system
	void BeginFrame();
	void EndFrame();

	void DxrUpdate();

	void DxrRender(); //!< Test method

	void Sent();

	DxrObject::Devices* GetDevicesObj() { return devices_.get(); }

	DxrObject::Command* GetCommandObj() { return command_.get(); }


	static DxrCommon* GetInstance();

private:

	//=========================================================================================
	// private variables
	//=========================================================================================

	std::unique_ptr<DxrObject::Devices>         devices_;
	std::unique_ptr<DxrObject::Command>         command_;
	std::unique_ptr<DxrObject::DescriptorHeaps> descriptorHeaps_;
	std::unique_ptr<DxrObject::SwapChain>       swapChain_;
	std::unique_ptr<DxrObject::Fence>           fence_;

	UINT backBufferIndex_;

	// object
	ComPtr<ID3D12Resource> vertexBuffer_;

	ComPtr<ID3D12Resource> blas_;
	ComPtr<ID3D12Resource> tlas_;
	DxrObject::Descriptor tlasDescriptor_;

	// objects
	PolygonMesh plane_;
	PolygonMesh cube_;

	// global
	ComPtr<ID3D12RootSignature> rootSignatureGlobal_;

	// loacl
	ComPtr<ID3D12RootSignature> rayGenRootSignature_;
	ComPtr<ID3D12RootSignature> modelRootSignature_;

	// shader
	static const LPCWSTR kShaderModel_;
	static const LPCWSTR kShaderFileName_;
	ComPtr<IDxcBlob> shaderBlob_;

	static const LPCWSTR kRayGenerationFunctionName_;
	static const LPCWSTR kClosestHitFunctionName_;
	static const LPCWSTR kMissFunctionName_;

	static const LPCWSTR kHitGroup_;

	// stateObject
	ComPtr<ID3D12StateObject> stateObject_;

	ComPtr<ID3D12Resource> outputResource_;
	DxrObject::Descriptor outputDescriptor_;

	ComPtr<ID3D12Resource> shaderTable_;
	D3D12_DISPATCH_RAYS_DESC dispatchRayDesc_;

	// constantBuffer
	ComPtr<ID3D12Resource> lightBuffer_;

	Vector3f rotate_ = { 0.0f, 0.0f, 0.0f };
	ComPtr<ID3D12Resource> rotateBuffer_;

	//=========================================================================================
	// private methods
	//=========================================================================================

	void CreateBLAS();

	void CreateTLAS();

	void CreateRootSignatureGlobal();

	void CreateShaderBlob();

	void CreateStateObject();

	void CreateResultBuffer(int32_t clientWidth, int32_t clientHeight);

	void CreateShaderTable(int32_t clientWidth, int32_t clientHeight);


	/// new

	void CreateObject();

	void CreateConstantBuffer();

	void CreateSceneBLAS();

	void CreateSceneTLAS();

	void DeployObjects(std::vector<D3D12_RAYTRACING_INSTANCE_DESC>& instanceDescs);

	void CreateGlobalRootSignature();

	void CreateLoaclRootSignatureRayGen();

	void CreateLoaclRootSignatureColosestHit();

	//=========================================================================================
	// create methods
	//=========================================================================================

	ComPtr<ID3D12Resource> CreateBuffer(
		size_t size, const void* data, D3D12_HEAP_TYPE type, D3D12_RESOURCE_FLAGS flags, const wchar_t* name);

	ComPtr<ID3D12Resource> CreateBuffer(
		size_t size, D3D12_RESOURCE_FLAGS flags, D3D12_RESOURCE_STATES initialState, D3D12_HEAP_TYPE heapType, const wchar_t* name = nullptr);

	DxrObject::Descriptor CreateStructuredSRV(
		ID3D12Resource* resource,
		UINT numElements, UINT firstElement, UINT stride);

	D3D12_RAYTRACING_GEOMETRY_DESC GetGeometryDesc(const PolygonMesh& mesh);

	AccelerationStructureBuffers CreateAccelerationStructure(
		const D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC& desc
	);

	uint8_t* WriteShaderRecord(uint8_t* dst, const PolygonMesh& mesh, UINT recordSize);

	UINT WriteGPUDescriptor(void* dst, const DxrObject::Descriptor& descriptor) {
		auto handle = descriptor.handleGPU;
		memcpy(dst, &handle, sizeof(handle));
		return UINT(sizeof(handle));
	}

};

////////////////////////////////////////////////////////////////////////////////////////////
// Create class
////////////////////////////////////////////////////////////////////////////////////////////
namespace Create {

	void Plane(std::vector<VertexDataDXR>& vertex, std::vector<UINT>& index);

	void Cube(std::vector<VertexDataDXR>& vertex, std::vector<UINT>& index);

}