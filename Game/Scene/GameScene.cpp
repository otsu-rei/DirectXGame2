#include "GameScene.h"

//-----------------------------------------------------------------------------------------
// include
//-----------------------------------------------------------------------------------------
// engine
#include <MyEngine.h>

////////////////////////////////////////////////////////////////////////////////////////////
// GameScene class methods
////////////////////////////////////////////////////////////////////////////////////////////

void GameScene::Run() {
	
	console_->Init();
	Init();

	MyEngine::TransitionProcessSingle();
	MyEngine::EnableTextures();

	////////////////////////////////////////////////////////////////////////////////////////////
	// メインループ
	////////////////////////////////////////////////////////////////////////////////////////////
	while (MyEngine::ProcessMessage() == 0) {

		MyEngine::BeginFrame();

		//=========================================================================================
		// 更新処理
		//=========================================================================================

		console_->Update();
		Update();

		MyEngine::TransitionProcess();
		MyEngine::BeginDraw();

		//=========================================================================================
		// オフスクリーン描画処理
		//=========================================================================================

		Draw();

		MyEngine::EndFrame();

	}

	Term();
	console_->Term();

}

//=========================================================================================
// private methods
//=========================================================================================

void GameScene::Init() {

	camera_ = std::make_unique<Camera3D>();
	MyEngine::camera3D = camera_.get();

	floor_ = std::make_unique<Floor>();
	player_ = std::make_unique<Player>();

}

void GameScene::Term() {
}

void GameScene::Update() {
	floor_->Update();
	player_->Update();
}

void GameScene::Draw() {

	//=========================================================================================
	// オフスクリーン描画処理
	//=========================================================================================

	{
		// todo: 複数へのtexture書き込みをさせる.
		MyEngine::BeginOffScreen(console_->GetSceneTexture());
		MyEngine::camera3D = console_->GetDebugCamera();


		floor_->Draw();
		player_->Draw();

		MyEngine::EndOffScreen();

		MyEngine::BeginOffScreen(MyEngine::GetTexture("offscreen"));
		MyEngine::camera3D = camera_.get();

		


		MyEngine::EndOffScreen();
	}

	MyEngine::TransitionProcess();

	//=========================================================================================
	// スクリーン描画処理
	//=========================================================================================

	{
		MyEngine::BeginScreenDraw();

	}

	/*
	 ImGuiの関係上、スクリーン描画は最後にする
	*/
}
