#include "DxrDescriptorHeaps.h"

//-----------------------------------------------------------------------------------------
// include
//-----------------------------------------------------------------------------------------
// DxrObject
#include <DxrDevices.h>
#include <DxrSwapChain.h>

#include <Logger.h>

////////////////////////////////////////////////////////////////////////////////////////////
// DescriptorHeaps class methods
////////////////////////////////////////////////////////////////////////////////////////////

void DxrObject::DescriptorHeaps::Init(Devices* devices) {

	// デバイスの取り出し
	ID3D12Device* device = devices->GetDevice();

	// descriptorの要素数を決定
	descriptorIndexSize_[RTV] = SwapChain::GetBufferCount();
	descriptorIndexSize_[SRV] = 128;
	descriptorIndexSize_[DSV] = 1;

	// 添え字の初期化
	for (int i = 0; i < DescriptorType::kDescriptorHeapCount; ++i) {
		descriptorIndexCount_[i] = 0;
	}

	// ディスクリプターヒープ[RTV]の生成
	{
		descriptorHeaps_[RTV] = DxObjectMethod::CreateDescriptorHeap(
			device,
			D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
			descriptorIndexSize_[RTV],
			false
		);

		Log("[DxrObject::DescriptorHeaps]: descriptorHeaps_[RTV] << Complete Create \n");
	}

	// ディスクリプターヒープ[SRV]の生成
	{
		descriptorHeaps_[SRV] = DxObjectMethod::CreateDescriptorHeap(
			device,
			D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
			descriptorIndexSize_[SRV],
			true
		);

		Log("[DxrObject::DescriptorHeaps]: descriptorHeaps_[SRV] << Complete Create \n");
	}

	// ディスクリプターヒープ[DSV]の生成
	{
		descriptorHeaps_[DSV] = DxObjectMethod::CreateDescriptorHeap(
			device,
			D3D12_DESCRIPTOR_HEAP_TYPE_DSV,
			descriptorIndexSize_[DSV],
			false
		);

		Log("[DxrObject::DescriptorHeaps]: descriptorHeaps_[DSV] << Complete Create \n");
	}

	// descriptorのサイズを取得
	descriptorSize_[RTV] = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	descriptorSize_[SRV] = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	descriptorSize_[DSV] = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
}

void DxrObject::DescriptorHeaps::Term() {
	for (int i = 0; i < kDescriptorHeapCount; ++i) {
		/*descriptorHeaps_[i]->Release();*/
		descriptorSize_[i] = 0;

		descriptorIndexSize_[i]  = 0;
		descriptorIndexCount_[i] = 0;

		/*descriptorVacantIndexs_[i].clear();*/
	}
}

const uint32_t DxrObject::DescriptorHeaps::GetDescriptorCurrentIndex(DescriptorType type) {

	assert(type < DescriptorType::kDescriptorHeapCount); //!< descrptorHeaps配列のオーバー

	// DescriptorTypeのindexを取得
	uint32_t typeIndex = static_cast<uint32_t>(type);

	//!< Queueの追加後
	//if (!descriptorVacantIndexs_[typeIndex].empty()) { //!< 空きindexがある場合
	//	// 先頭の空きindexを返却
	//	uint32_t result = descriptorVacantIndexs_[typeIndex].front();
	//	// 先頭の空きインデックスを渡すので先頭をpop
	//	descriptorVacantIndexs_[typeIndex].pop_front();
	//	return result;
	//}

	assert(descriptorIndexCount_[typeIndex] < descriptorIndexSize_[typeIndex]); //!< 作成した分の配列サイズを超えている

	// 現在のindexCountを返却
	uint32_t result = descriptorIndexCount_[typeIndex];

	// 取得するのでインクリメント
	descriptorIndexCount_[typeIndex]++;

	return result;
}

DxrObject::Descriptor DxrObject::DescriptorHeaps::GetCurrentDescriptor(DescriptorType type) {
	Descriptor result;

	result.index = GetDescriptorCurrentIndex(type);
	result.handleCPU = GetCPUDescriptorHandle(type, result.index);
	result.handleGPU = GetGPUDescriptorHandle(type, result.index);
	result.type = type;

	return result;
}
