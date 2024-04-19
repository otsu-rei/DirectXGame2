#pragma once

//-----------------------------------------------------------------------------------------
// include
//-----------------------------------------------------------------------------------------
// directX
#include <d3d12.h>
#include <dxgi1_6.h>

// c++
#include <cstdint>

// ComPtr
#include <ComPtr.h>

//-----------------------------------------------------------------------------------------
// comment
//-----------------------------------------------------------------------------------------
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

//-----------------------------------------------------------------------------------------
// forward
//-----------------------------------------------------------------------------------------
class WinApp;

////////////////////////////////////////////////////////////////////////////////////////////
// DxrObject namespace
////////////////////////////////////////////////////////////////////////////////////////////
namespace DxrObject {

	//-----------------------------------------------------------------------------------------
	// DxObject forward
	//-----------------------------------------------------------------------------------------
	class Devices;
	class Command;
	class DescriptorHeaps;

	////////////////////////////////////////////////////////////////////////////////////////////
	// class SwapChain
	////////////////////////////////////////////////////////////////////////////////////////////
	class SwapChain {
	public:

		//=========================================================================================
		// public methods
		//=========================================================================================

		//! @brief 初期化
		//! 
		//! @param[in] devices         DxObject::Devices
		//! @param[in] command         DxObject::Command
		//! @param[in] descriptorHeaps DxObject::DescriptorHeaps
		//! @param[in] winApp          WinApp
		//! @param[in] clientWidth     クライアント横幅
		//! @param[in] clientHeight    クライアント縦幅
		SwapChain(
			Devices* devices, Command* command, DescriptorHeaps* descriptorHeaps,
			WinApp* winApp, int32_t clientWidth, int32_t clientHeight
		);

		//! @brief デストラクタ
		~SwapChain();

		//! @brief 初期化
		//! 
		//! @param[in] devices         DxObject::Devices
		//! @param[in] command         DxObject::Command
		//! @param[in] descriptorHeaps DxObject::DescriptorHeaps
		//! @param[in] winApp          WinApp
		//! @param[in] clientWidth     クライアント横幅
		//! @param[in] clientHeight    クライアント縦幅
		void Init(
			Devices* devices, Command* command, DescriptorHeaps* descriptorHeaps,
			WinApp* winApp, int32_t clientWidth, int32_t clientHeight
		);

		//! @brief 終了処理
		void Term();

		//! @brief Presentを実行
		void Present(UINT SyncInterval, UINT Flags);

		//! @brief スワップチェインを取得
		//! 
		//! @return スワップチェインを返却
		IDXGISwapChain4* GetSwapChain() const { return swapChain_.Get(); }

		//! @brief トランジションバリアを取得
		//! 
		//! @param[in] backBufferIndex
		//! @param[in] stateBefore
		//! @param[in] stateAfter
		//! 
		//! @return トランジションバリアを返却
		D3D12_RESOURCE_BARRIER* GetTransitionBarrier(
			UINT backBufferIndex,
			D3D12_RESOURCE_STATES stateBefore, D3D12_RESOURCE_STATES stateAfter
		);

		//! @brief handleCPU_RTVを取得
		//! 
		//! @param[in] backBufferIndex 
		//! 
		//! @return handleCPU_RTVを返却
		const D3D12_CPU_DESCRIPTOR_HANDLE& GetHandleCPU_RTV(UINT backBufferIndex) const { return handleCPU_RTV_[backBufferIndex]; }

		ID3D12Resource* GetResource(uint32_t backBufferIndex) const { return swapChainResource_[backBufferIndex].Get(); }

		static const uint32_t GetBufferCount() { return kBufferCount_; }

	private:

		//=========================================================================================
		// private variables
		//=========================================================================================
		static const uint32_t kBufferCount_ = 2;

		ComPtr<IDXGISwapChain4> swapChain_;
		ComPtr<ID3D12Resource>  swapChainResource_[kBufferCount_];

		D3D12_CPU_DESCRIPTOR_HANDLE handleCPU_RTV_[kBufferCount_];

		D3D12_RESOURCE_BARRIER barrier_;

	};
}