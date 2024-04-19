#include <MyEngine.h> // sxavenger engine
#include <DirectXCommon.h>

//-----------------------------------------------------------------------------------------
// include
//-----------------------------------------------------------------------------------------
/// lib
#include <Environment.h>
#include <ExecutionSpeed.h>
// Geometry
#include <Vector4.h>
#include <Matrix4x4.h>
#include <Vector2.h>

// Light
#include <Light.h>

// c++
#include <list>
#include <memory>

// Game
#include <Logger.h>

#include "DxrCommon.h"

////////////////////////////////////////////////////////////////////////////////////////////
// メイン関数
////////////////////////////////////////////////////////////////////////////////////////////
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {

	//=========================================================================================
	// 初期化
	//=========================================================================================
	MyDxrEngine::Initialize(kWindowWidth, kWindowHeight, kWindowTitle);

	////////////////////////////////////////////////////////////////////////////////////////////
	// メインループ
	////////////////////////////////////////////////////////////////////////////////////////////
	while (MyDxrEngine::ProcessMessage() == 0) {

		/*MyEngine::BeginFrame();*/
		/*MyDxrEngine::BeginFrame();*/

		MyDxrEngine::GetDxrCommon()->DxrRender();

		//=========================================================================================
		// 更新処理
		//=========================================================================================

		/*ImGui::Begin("system");
		ImGui::Text("speed(s): %.6f", ExecutionSpeed::freamsParSec_);
		ImGui::Text("FPS: %.1f", 1.0f / ExecutionSpeed::freamsParSec_);
		ImGui::End();

		ImGui::Begin("main");
		ImGui::End();*/

		

		//=========================================================================================
		// 描画処理
		//=========================================================================================
		
		/*MyEngine::EndFrame();*/
		/*MyDxrEngine::EndFrame();*/
	}

	MyDxrEngine::Finalize();
	
	return 0;

}