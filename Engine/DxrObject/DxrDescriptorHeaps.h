#pragma once

//-----------------------------------------------------------------------------------------
// include
//-----------------------------------------------------------------------------------------
// directX
#include <d3d12.h>
#include <dxgi1_6.h>

// c++
#include <cstdint>

// c++
#include <DxObjectMethod.h>
// 特に変更点が無ければDxObjectMethodを使用

// ComPtr
#include <ComPtr.h>

//-----------------------------------------------------------------------------------------
// comment
//-----------------------------------------------------------------------------------------
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

////////////////////////////////////////////////////////////////////////////////////////////
// DxrObject namespace
////////////////////////////////////////////////////////////////////////////////////////////
namespace DxrObject {

	//-----------------------------------------------------------------------------------------
	// DxObject forward
	//-----------------------------------------------------------------------------------------
	class Devices;

	////////////////////////////////////////////////////////////////////////////////////////////
	// DescriptorType enum
	////////////////////////////////////////////////////////////////////////////////////////////
	enum DescriptorType {
		RTV, SRV, DSV, //!< 作成するDescriptorの種類

		kDescriptorHeapCount
	};

	////////////////////////////////////////////////////////////////////////////////////////////
	// Descriptor structure
	////////////////////////////////////////////////////////////////////////////////////////////
	struct Descriptor {
		uint32_t index;
		DescriptorType type;
		D3D12_CPU_DESCRIPTOR_HANDLE handleCPU;
		D3D12_GPU_DESCRIPTOR_HANDLE handleGPU;
	};

	////////////////////////////////////////////////////////////////////////////////////////////
	// DescriptorHeaps class
	////////////////////////////////////////////////////////////////////////////////////////////
	class DescriptorHeaps {
	public:

		//=========================================================================================
		// public methods
		//=========================================================================================

		//! @brief コンストラクタ
		//! 
		//! @param[in] devices DxObject::Devices
		DescriptorHeaps(Devices* devices) { Init(devices); }

		//! @brief デストラクタ
		~DescriptorHeaps() { Term(); }

		//! @brief 初期化処理
		//! 
		//! @param[in] devices DxObject::Devices
		void Init(Devices* devices);

		//! @brief 終了処理
		void Term();

		//! @breif 使用できるDescriptorsのindexを取得
		//! 
		//! @param[in] type DescriptorType
		//! 
		//! @return 使用できるDescriptorsのindexを返却
		const uint32_t GetDescriptorCurrentIndex(DescriptorType type);

		//! @breif 使用できるDescriptorsのindexを取得
		//! 
		//! @param[in] type DescriptorType
		//! 
		//! @return 使用できるDescriptorsのindexを返却
		Descriptor GetCurrentDescriptor(DescriptorType type);

		//! @brief CPUDescriptorHandleの取得
		//! 
		//! @param[in] type  DescriptorType
		//! @param[in] index
		//! 
		//! @return CPUDescriptorHandleの返却
		D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle(DescriptorType type, uint32_t index) const {
			assert(type < DescriptorType::kDescriptorHeapCount);

			return DxObjectMethod::GetCPUDescriptorHandle(
				descriptorHeaps_[type].Get(),
				descriptorSize_[type],
				index
			);
		}

		//! @brief GPUDescriptorHandleの取得
		//! 
		//! @param[in] type  DescriptorType
		//! @param[in] index
		//! 
		//! @return GPUDescriptorHandleの返却
		D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle(DescriptorType type, uint32_t index) const {
			assert(type < DescriptorType::kDescriptorHeapCount);

			return DxObjectMethod::GetGPUDescriptorHandle(
				descriptorHeaps_[type].Get(),
				descriptorSize_[type],
				index
			);
		}

		ID3D12DescriptorHeap* GetDescriptorHeap(DescriptorType type) const {
			return descriptorHeaps_[type].Get();
		}

	private:

		//=========================================================================================
		// private variables
		//=========================================================================================

		ComPtr<ID3D12DescriptorHeap> descriptorHeaps_[DescriptorType::kDescriptorHeapCount];
		uint32_t                     descriptorSize_[DescriptorType::kDescriptorHeapCount]; // constにしたい

		uint32_t descriptorIndexSize_[DescriptorType::kDescriptorHeapCount];
		uint32_t descriptorIndexCount_[DescriptorType::kDescriptorHeapCount];

		// deque まだ必要ないので追加してない

	};

}