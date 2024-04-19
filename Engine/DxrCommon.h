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

// directX
#include <d3d12.h>
#include <dxgi1_6.h>
#include <dxcapi.h>

// c++
#include <cassert>

#include <ComPtr.h>

//-----------------------------------------------------------------------------------------
// comment
//-----------------------------------------------------------------------------------------
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxcompiler.lib")

////////////////////////////////////////////////////////////////////////////////////////////
// PolygonObject structure
////////////////////////////////////////////////////////////////////////////////////////////
struct PolygonObject {
	
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

	void BeginFrame();

	void EndFrame();

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

	// global
	ComPtr<ID3D12RootSignature> rootSignatureGlobal_;

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

};