#pragma once

#include <DxrDevices.h>
#include <DxObjectMethod.h>
#include <cassert>

namespace DxrObject {

	////////////////////////////////////////////////////////////////////////////////////////////
	// BufferResource class
	////////////////////////////////////////////////////////////////////////////////////////////
	template <typename T>
	class BufferResource {
	public:

		//=========================================================================================
		// public methods
		//=========================================================================================

		//! @breif コンストラクタ
		//! 
		//! @param[in] devices   DxObject::Devices
		//! @param[in] indexSize 配列サイズ
		BufferResource(DxrObject::Devices* devices, uint32_t indexSize) {
			Init(devices, indexSize);
		}

		//! @brief デストラクタ
		~BufferResource() { Term(); }

		//! @brief 初期化処理
		//! 
		//! @param[in] devices DxObject::Devices
		void Init(DxrObject::Devices* devices, uint32_t indexSize);

		//! @brief 終了処理
		void Term();

		//! @brief BufferResourceを取得
		//! 
		//! @return BufferResourceを返却
		ID3D12Resource* GetResource() const {
			return resource_.Get();
		}

		//! @brief GPUAddressを取得
		//! 
		//! @return GPUAddressを返却
		const D3D12_GPU_VIRTUAL_ADDRESS GetGPUVirtualAddress() const {
			return resource_->GetGPUVirtualAddress();
		}

		//! @brief VertexBufferを取得
		//! 
		//! @return VertexBufferを返却
		const D3D12_VERTEX_BUFFER_VIEW GetVertexBufferView() const {
			D3D12_VERTEX_BUFFER_VIEW result = {};
			result.BufferLocation = resource_->GetGPUVirtualAddress();
			result.SizeInBytes    = sizeof(T) * indexSize_;
			result.StrideInBytes  = sizeof(T);

			return result;
		}

		void Memcpy(const T* value) {
			memcpy(dataArray_, value, sizeof(T) * indexSize_);
		}

		//! @breif 配列のサイズを獲得
		const uint32_t GetSize() const { return indexSize_; }

		//=========================================================================================
		// operator
		//=========================================================================================

		T& operator[](uint32_t index) {
			assert(CheckIndex(index));

			return dataArray_[index];
		}

	private:

		//=========================================================================================
		// private variables
		//=========================================================================================

		ComPtr<ID3D12Resource> resource_;

		T* dataArray_;
		uint32_t indexSize_;

		bool CheckIndex(uint32_t index);

	};


}

////////////////////////////////////////////////////////////////////////////////////////////
// BufferResource class methods
////////////////////////////////////////////////////////////////////////////////////////////

template<typename T>
void DxrObject::BufferResource<T>::Init(DxrObject::Devices* devices, uint32_t indexSize) {

	// deviceを取り出す
	ID3D12Device* device = devices->GetDevice();

	indexSize_ = indexSize;

	// 配列分のBufferResourceを生成
	resource_ = DxObjectMethod::CreateBufferResource(
		device,
		sizeof(T) * indexSize_
	);

	// resourceをマッピング
	resource_->Map(0, nullptr, reinterpret_cast<void**>(&dataArray_));
}

template<typename T>
void DxrObject::BufferResource<T>::Term() {
	resource_->Release();
}


template<typename T>
bool DxrObject::BufferResource<T>::CheckIndex(uint32_t index) {
	if (index > indexSize_ - 1) {
		return false;
	}

	return true;
}