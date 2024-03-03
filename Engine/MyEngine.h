#pragma once

//-----------------------------------------------------------------------------------------
// include
//-----------------------------------------------------------------------------------------
#include <windows.h>
#include <cstdint>
#include <string>

// DxObject
#include <DxDevices.h>
#include <DxDescriptorHeaps.h>

// Camera
#include <Camera3D.h>
#include <Camera2D.h>

//-----------------------------------------------------------------------------------------
// forward
//-----------------------------------------------------------------------------------------
class DirectXCommon;
class TextureManager;

enum PipelineType {
	TEXTURE,
	POLYGON
};

////////////////////////////////////////////////////////////////////////////////////////////
// MyEngine class
////////////////////////////////////////////////////////////////////////////////////////////
class MyEngine {
public:

	//=========================================================================================
	// public methods
	//=========================================================================================

	//! @brief 初期化処理
	//! 
	//! @param[in] width        ウィンドウ横幅
	//! @param[in] height       ウィンドウ縦幅
	//! @param[in] kWindowTitle ウィンドウタイトル
	static void Initialize(int32_t kWindowWidth, int32_t kWindowHeight, const char* kWindowTitle);

	//! @brief 終了処理
	static void Finalize();

	//! @brief フレームの開始
	static void BeginFrame();

	//! @brief フレームの終了
	static void EndFrame();

	//! @brief プロセスメッセージを取得
	//! 
	//! @retval 1 ゲーム終了
	//! @retval 0 ゲーム継続
	static int ProcessMessage();

	// TODO: あんましたくない
	static void SetPipeLineState(PipelineType type);
	static ID3D12GraphicsCommandList* GetCommandList();

	static DxObject::Devices* GetDevice();
	static DirectXCommon* GetDxCommon();

	static TextureManager* GetTextureManager();

	static const D3D12_GPU_DESCRIPTOR_HANDLE& GetTextureHandleGPU(const std::string& textureKey);

	//=========================================================================================
	// public variables
	//=========================================================================================
	
	static Camera3D* camera3D_;

	static Camera2D* camera2D_;

private:
};