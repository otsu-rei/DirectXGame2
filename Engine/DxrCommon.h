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

};