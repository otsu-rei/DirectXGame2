#include "MyEngine.h"

//-----------------------------------------------------------------------------------------
// include
//-----------------------------------------------------------------------------------------
#include <Logger.h>

// origin
#include <WinApp.h>
#include <DirectXCommon.h>
#include <ImGuiManager.h>
#include <TextureManager.h>

// test origin
#include <DxrCommon.h>

#include <ComPtr.h>

#include <ExecutionSpeed.h>

////////////////////////////////////////////////////////////////////////////////////////////
// namespace -anonymouse-
////////////////////////////////////////////////////////////////////////////////////////////
namespace {

	//-----------------------------------------------------------------------------------------
	// 汎用機能
	//-----------------------------------------------------------------------------------------
	WinApp* sWinApp = nullptr;                 //!< WindowApp system
	DirectXCommon* sDirectXCommon = nullptr;   //!< DirectX12 system
	ImGuiManager* sImGuiManager = nullptr;     //!< ImGui system
	TextureManager* sTextureManager = nullptr; //!< TextureManager system

	
}

////////////////////////////////////////////////////////////////////////////////////////////
// MyEngine class
////////////////////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------------------
// static variables
//-----------------------------------------------------------------------------------------
Camera3D* MyEngine::camera3D_ = nullptr;
Camera2D* MyEngine::camera2D_ = nullptr;

//-----------------------------------------------------------------------------------------
// method
//-----------------------------------------------------------------------------------------

void MyEngine::Initialize(int32_t kWindowWidth, int32_t kWindowHeight, const char* kWindowTitle) {
	CoInitializeEx(0, COINIT_MULTITHREADED);

	// ウィンドウの生成
	{
		sWinApp = WinApp::GetInstance();

		std::wstring titleString = ToWstring(kWindowTitle);
		sWinApp->CreateGameWindow(kWindowWidth, kWindowHeight, titleString.c_str());
	}

	// DirectX12 の初期化
	{
		sDirectXCommon = DirectXCommon::GetInstance();
		sDirectXCommon->Init(sWinApp, kWindowWidth, kWindowHeight);
	}

	// ImGuiManager の初期化
	{
		sImGuiManager = ImGuiManager::GetInstaince();
		sImGuiManager->Init(sWinApp, sDirectXCommon);
	}

	// TextureManager の初期化
	{
		sTextureManager = TextureManager::GetInstance();
		sTextureManager->Init(sDirectXCommon);
	}
}

void MyEngine::Finalize() {
	sTextureManager->Term();
	sTextureManager = nullptr;

	sImGuiManager->Term();
	sImGuiManager = nullptr;

	sDirectXCommon->Term();
	sDirectXCommon = nullptr;

	sWinApp->Term();
	sWinApp = nullptr;

	CoUninitialize();
}

void MyEngine::BeginFrame() {
	ExecutionSpeed::Begin();
	sDirectXCommon->BeginFrame();
	sImGuiManager->Begin();
}

void MyEngine::EndFrame() {
	sDirectXCommon->GetDescriptorsObj()->Debug();

	sImGuiManager->End();
	sDirectXCommon->EndFrame();
	ExecutionSpeed::End();
}

int MyEngine::ProcessMessage() {
	return sWinApp->ProcessMessage() ? 1 : 0;
}

void MyEngine::SetPipelineType(PipelineType pipelineType) {
	sDirectXCommon->SetPipelineType(pipelineType);
}

void MyEngine::SetBlendMode(BlendMode mode) {
	sDirectXCommon->SetBlendMode(mode);
}

void MyEngine::SetPipelineState() {
	sDirectXCommon->SetPipelineState();
}

ID3D12GraphicsCommandList* MyEngine::GetCommandList() {
	assert(sDirectXCommon != nullptr);
	return sDirectXCommon->GetCommandList();
}

DxObject::Devices* MyEngine::GetDevicesObj() {
	assert(sDirectXCommon != nullptr);
	return sDirectXCommon->GetDeviceObj();
}

DirectXCommon* MyEngine::GetDxCommon() {
	assert(sDirectXCommon != nullptr);
	return sDirectXCommon;
}

TextureManager* MyEngine::GetTextureManager() {
	assert(sTextureManager != nullptr);
	return sTextureManager;
}

const D3D12_GPU_DESCRIPTOR_HANDLE& MyEngine::GetTextureHandleGPU(const std::string& textureKey) {
	assert(sTextureManager != nullptr);
	return sTextureManager->GetHandleGPU(textureKey);
}


////////////////////////////////////////////////////////////////////////////////////////////
// namespace -anonymouse- // test function
////////////////////////////////////////////////////////////////////////////////////////////
namespace {
	//-----------------------------------------------------------------------------------------
	// テスト機能
	//-----------------------------------------------------------------------------------------
	DxrCommon* sDxrCommon = nullptr;
}

////////////////////////////////////////////////////////////////////////////////////////////
// MyDxrEngine class
////////////////////////////////////////////////////////////////////////////////////////////

void MyDxrEngine::Initialize(int32_t kWindowWidth, int32_t kWindowHeight, const char* kWindowTitle) {
	CoInitializeEx(0, COINIT_MULTITHREADED);

	// ウィンドウの生成
	{
		sWinApp = WinApp::GetInstance();
		std::wstring titleString = ToWstring(kWindowTitle);
		sWinApp->CreateGameWindow(kWindowWidth, kWindowHeight, titleString.c_str());
	}

	// DxrCommon初期化
	{
		sDxrCommon = DxrCommon::GetInstance();
		sDxrCommon->Init(sWinApp, kWindowWidth, kWindowHeight);
	}
}

void MyDxrEngine::Finalize() {
	sWinApp->Term();
	sWinApp = nullptr;

	sDxrCommon->Term();
	sDxrCommon = nullptr;

	CoUninitialize();
}

void MyDxrEngine::BeginFrame() {
	sDxrCommon->BeginFrame();
}

void MyDxrEngine::EndFrame() {
	sDxrCommon->EndFrame();
}

int MyDxrEngine::ProcessMessage() {
	return sWinApp->ProcessMessage() ? 1 : 0;
}

DxrObject::Devices* MyDxrEngine::GetDevicesObj() {
	assert(sDxrCommon);
	return sDxrCommon->GetDevicesObj();
}

DxrObject::Command* MyDxrEngine::GetCommandObj() {
	assert(sDxrCommon);
	return sDxrCommon->GetCommandObj();
}

DxrCommon* MyDxrEngine::GetDxrCommon() {
	assert(sDxrCommon);
	return sDxrCommon;
}
