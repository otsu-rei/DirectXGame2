#pragma once

//*****************************************************************************************
// todo: imguiのスタイル変更, gameSceneのwindowの追加
//*****************************************************************************************

//-----------------------------------------------------------------------------------------
// include
//-----------------------------------------------------------------------------------------
// D3D12
#include <d3d12.h>

// c++
#include <deque>
#include <string>
#include <list>
#include <memory>
#include <filesystem>

// geometry
#include <Vector4.h>

#include <Camera3D.h>

//-----------------------------------------------------------------------------------------
// forward
//-----------------------------------------------------------------------------------------
class Attribute;
class DirectXCommon;
class Texture;

////////////////////////////////////////////////////////////////////////////////////////////
// Console class
////////////////////////////////////////////////////////////////////////////////////////////
class Console {
public:

	//=========================================================================================
	// public methods
	//=========================================================================================

	static Console* GetInstance();

	//! @brief 初期化処理
	void Init();

	//! @brief 終了処理
	void Term();

	//! @brief 更新処理
	void Update();

	//! @brief ログをセット
	//! 
	//! @param[in] log   出力したいログ
	//! @param[in] color 文字の色
	void SetLog(const std::string& log, const Vector4f& color = {1.0f, 1.0f, 1.0f, 1.0f});

	void SetAttribute(Attribute* obj) {
		Outliner_.push_back(obj);
	}

	Texture* GetSceneTexture() const { return sceneTexture_; }

	Camera3D* GetDebugCamera() const { return debugCamera_.get(); }

	//
	// test function
	//

	void OutputRayTracingResult(const D3D12_GPU_DESCRIPTOR_HANDLE& srvHandleGPU);

	void OutputTexture(const std::string& name, const D3D12_GPU_DESCRIPTOR_HANDLE& srvHandleGPU);

	//=========================================================================================
	// public variables
	//=========================================================================================

	static const Vector4f commentOutColor;
	static const Vector4f errorColor;
	static const Vector4f warningColor;
	static const Vector4f defaultColor;

private:

	////////////////////////////////////////////////////////////////////////////////////////////
	// LogData structture
	////////////////////////////////////////////////////////////////////////////////////////////
	struct LogData {
		std::string log;
		Vector4f color;
	};

	//=========================================================================================
	// private variables
	//=========================================================================================

	DirectXCommon* dxCommon_;

	// 
	bool isOutputEngineUI_ = true;
	bool isFix_ = true;

	int windowFlags = 0;

	// scenes
	bool isFocusDebugScene_ = false;
	bool isFocusGameScene_  = false;

	std::unique_ptr<Camera3D> debugCamera_;
	Texture* sceneTexture_;

	// logs
	static const int32_t kMaxLogData_ = 30;
	std::deque<LogData> logDatas_;

	// Attribute
	std::list<Attribute*> Outliner_; //!< attributes
	Attribute* selectedAttribute_;

	//=========================================================================================
	// private methods
	//=========================================================================================

	void OutputMain();

	void OutputScene();
	void OutputGame();

	void OutputLog();
	void OutputPrintf();

	void OutputPerformance();

	void OutputOutliner();

	void OutputSystem();
	void OutputAssets();

	void SetTextureImGui(const D3D12_GPU_DESCRIPTOR_HANDLE& texture);

};

void ListFilesAndDirectories(const std::filesystem::path& directory);
