#pragma once

//-----------------------------------------------------------------------------------------
// include
//-----------------------------------------------------------------------------------------
// directX
#include <d3d12.h>
#include <dxgi1_6.h>

// c++
#include <cstdint>
#include <cassert>

// ComPtr
#include <ComPtr.h>

//-----------------------------------------------------------------------------------------
// comment
//-----------------------------------------------------------------------------------------
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

////////////////////////////////////////////////////////////////////////////////////////////
// DxObject namespace
////////////////////////////////////////////////////////////////////////////////////////////
namespace DxObject {

	//-----------------------------------------------------------------------------------------
	// DxObject forward
	//-----------------------------------------------------------------------------------------
	class Devices;
	class Fence;

	////////////////////////////////////////////////////////////////////////////////////////////
	// Command class
	////////////////////////////////////////////////////////////////////////////////////////////
	class Command {
	public:

		//=========================================================================================
		// public methods
		//=========================================================================================

		//! @brief コンストラクタ
		//! 
		//! @param[in] devices DxObject::Devices
		Command(Devices* device);

		//! @brief デストラクタ
		~Command();

		//! @brief 初期化
		//! 
		//! @param[in] devices DxObject::Devices
		void Init(Devices* device);

		//! @brief 終了処理
		void Term();

		//! @brief commandListをクローズし, Queueで実行
		void Close();

		//! @brief commandAllocatorのすべてをリセット, commandListをリセット
		void ResetAll();

		//! @brief backAllocatorの更新 & commandListをbackAllocatorにリセット
		void ResetBackAllocator();

		//! @brief コマンドリストを取得
		//! 
		//! @return コマンドリストを返却
		auto GetCommandList() const { return commandList_.Get(); }

		//! @brief コマンドキューを取得
		//! 
		//! @return コマンドキューを返却
		ID3D12CommandQueue* GetCommandQueue() const { return commandQueue_.Get(); }

		//! @brief Signalを実行
		//! 
		//! @param[in] fences DxObject::Fence
		void Signal(Fence* fences);

	private:

		//=========================================================================================
		// private variables
		//=========================================================================================

		static const uint32_t kAllocatorCount = 2;

		ComPtr<ID3D12CommandQueue>         commandQueue_;
		ComPtr<ID3D12CommandAllocator>     commandAllocator_[kAllocatorCount];
		ComPtr<ID3D12GraphicsCommandList6> commandList_;

		uint32_t backAllocatorIndex_ = 0;

		/*
			backAllocator  -> commandListを積んでるAllocator
			frontAllocator -> 実行中のallocator
		*/

	};
}