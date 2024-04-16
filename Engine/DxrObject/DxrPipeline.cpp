#include "DxrPipeline.h"

//-----------------------------------------------------------------------------------------
// include
//-----------------------------------------------------------------------------------------
#include <DxrDevices.h>
#include <DxrObjectMethods.h>

#include <Logger.h>
#include <cassert>

//-----------------------------------------------------------------------------------------
// using
//-----------------------------------------------------------------------------------------
using namespace DxrObjectMethod;

////////////////////////////////////////////////////////////////////////////////////////////
// GlobalRootSignature class methods
////////////////////////////////////////////////////////////////////////////////////////////

void DxrObject::GlobalRootSignature::Init(Devices* devices) {
	
	// deviceの取り出し
	ID3D12Device5* device = devices->GetDevice();

	D3D12_ROOT_SIGNATURE_DESC desc = {};
	globalRootSignature_ = CreateRootSignature(device, desc);

	// loacl
	D3D12_DESCRIPTOR_RANGE range[2] = {};
	range[0].BaseShaderRegister = 0;
	range[0].NumDescriptors = 1;
	range[0].RegisterSpace = 0;
	range[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
	range[0].OffsetInDescriptorsFromTableStart = 0;

	range[1].BaseShaderRegister = 0;
	range[1].NumDescriptors = 1;
	range[1].RegisterSpace = 0;
	range[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	range[1].OffsetInDescriptorsFromTableStart = 1;

	CD3DX12_ROOT_PARAMETER param = {};
	param.InitAsDescriptorTable(param, 2, range);

	D3D12_ROOT_SIGNATURE_DESC localDesc = {};
	localDesc.NumParameters = 1;
	localDesc.pParameters = &param;
	localDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE;

	localRootSignature_ = CreateRootSignature(device, localDesc);
	

}

void DxrObject::GlobalRootSignature::Term() {
}
