#pragma once

//-----------------------------------------------------------------------------------------
// include
//-----------------------------------------------------------------------------------------
// c++
#include <memory>
#include <array>
#include <cassert>

// DxObject
#include <DxBlendState.h>
#include <DxShaderBlob.h>
#include <DxRootSignature.h>
#include <DxPipelineState.h>

////////////////////////////////////////////////////////////////////////////////////////////
// PipelineType enum
////////////////////////////////////////////////////////////////////////////////////////////
enum PipelineType {
	FLOOR,
	DEFAULT,
	PRIMITIVE,
	BLASRENDER,
	GAUSSIANBLUR,
	PLAYER,

	kCountOfPipeline
};

////////////////////////////////////////////////////////////////////////////////////////////
// DxObject namespace
////////////////////////////////////////////////////////////////////////////////////////////
namespace DxObject {

	//-----------------------------------------------------------------------------------------
	// forward
	//-----------------------------------------------------------------------------------------
	class Devices;
	class Command;

	////////////////////////////////////////////////////////////////////////////////////////////
	// PipelineManager class
	////////////////////////////////////////////////////////////////////////////////////////////
	class PipelineManager {
	public:

		//=========================================================================================
		// public methods
		//=========================================================================================
		
		//! @brief コンストラクタ
		//! 
		//! @param[in] devices      DxObject::Devices
		//! @param[in] command      DxObject::Command
		//! @param[in] blendState   DxObject::BlendState
		//! @param[in] clientWidth  クライアント領域横幅
		//! @param[in] clientHeight クライアント領域縦幅
		PipelineManager(
			Devices* devices, Command* command, BlendState* blendState,
			int32_t clientWidth, int32_t clientHeight
		);

		//! @brief デストラクタ
		~PipelineManager() { Term(); }

		//! @brief 初期化処理
		//! 
		//! @param[in] devices      DxObject::Devices
		//! @param[in] command      DxObject::Command
		//! @param[in] blendState   DxObject::BlendState
		//! @param[in] clientWidth  クライアント領域横幅
		//! @param[in] clientHeight クライアント領域縦幅
		void Init(
			Devices* devices, Command* command, BlendState* blendState,
			int32_t clientWidth, int32_t clientHeight
		);

		//! @brief 終了処理
		void Term();

		//! @brief pipelineTypeの設定
		void SetPipelineType(PipelineType type) {
			pipelineType_ = type;
		}

		//! @brief blendModeの設定
		void SetBlendMode(BlendMode mode) {
			blendMode_ = mode;
		}

		//! @brief pipelineを生成
		void CreatePipeline();

		//! @brief commandListにPipelineを設定
		void SetPipeline();

	private:

		////////////////////////////////////////////////////////////////////////////////////////////
		// PipelineMenber structure
		////////////////////////////////////////////////////////////////////////////////////////////
		struct PipelineMenber {
			std::unique_ptr<ShaderBlob>    shaderBlob;
			std::unique_ptr<RootSignature> rootSignature;

			//! @brief Reset処理
			void Reset() {
				shaderBlob.reset();
				rootSignature.reset();
			}
		};

		////////////////////////////////////////////////////////////////////////////////////////////
		// PipelineData structure
		////////////////////////////////////////////////////////////////////////////////////////////
		struct PipelineData {
			std::unique_ptr<PipelineState> pipeline;
			BlendMode settingBlendMode_;
		};

		//=========================================================================================
		// private variables
		//=========================================================================================

		// adapter
		Devices* devices_ = nullptr;
		Command* command_ = nullptr;

		// blendState
		BlendState* blendState_ = nullptr;
		BlendMode   blendMode_  = BlendMode::kBlendModeNormal;

		// pipeline
		std::array<PipelineMenber, PipelineType::kCountOfPipeline> pipelineMenbers_;
		PipelineType                                               pipelineType_ = static_cast<PipelineType>(0);

		// pipelines
		std::array<PipelineData, PipelineType::kCountOfPipeline> pipelines_;
		//!< array[PipelineType]

		// viewports
		D3D12_VIEWPORT viewport_;
		D3D12_RECT     scissorRect_;

		//=========================================================================================
		// private methods
		//=========================================================================================

		void CreatePipelineTable();

	};



}