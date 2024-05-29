#include <MyEngine.h> // sxavenger engine
#include "DirectXCommon.h"

//-----------------------------------------------------------------------------------------
// include
//-----------------------------------------------------------------------------------------
// c++
#include <memory>

// Enviorment
#include <Environment.h>

// GameScene
#include <GameScene.h>

////////////////////////////////////////////////////////////////////////////////////////////
// メイン関数
////////////////////////////////////////////////////////////////////////////////////////////
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {

	//=========================================================================================
	// 初期化
	//=========================================================================================
	MyEngine::Initialize(kWindowWidth, kWindowHeight, kWindowTitle);

	std::unique_ptr<GameScene> gameScene = std::make_unique<GameScene>();

	gameScene->Run();

	gameScene.reset();

	MyEngine::Finalize();
	return 0;

}