#include "DxrObjectMethods.h"

#include <Logger.h>
#include <cassert>

ComPtr<ID3D12Resource> DxrObjectMethod::CreateBufferResource(
	ID3D12Device5* device,
	uint64_t size,
	D3D12_RESOURCE_FLAGS flag,
	D3D12_RESOURCE_STATES initalizeState,
	const D3D12_HEAP_PROPERTIES& prop) {

	D3D12_RESOURCE_DESC bufferDesc
		= CD3DX12_RESOURCE_DESC::Buffer(sizeof(size), flag);
	
	ComPtr<ID3D12Resource> buffer = nullptr;
	auto hr = device->CreateCommittedResource(
		&prop,
		D3D12_HEAP_FLAG_NONE,
		&bufferDesc,
		initalizeState,
		nullptr,
		IID_PPV_ARGS(&buffer)
	);

	assert(SUCCEEDED(hr));

	return buffer;
}

ComPtr<ID3D12RootSignature> DxrObjectMethod::CreateRootSignature(
	ID3D12Device5* device,
	const D3D12_ROOT_SIGNATURE_DESC& desc) {

	ComPtr<ID3D12RootSignature> result;

	// blob
	ComPtr<ID3DBlob> blob, errorBlob;

	auto hr = D3D12SerializeRootSignature(
		&desc,
		D3D_ROOT_SIGNATURE_VERSION_1,
		&blob,
		&errorBlob
	);

	if (FAILED(hr)) {
		Log(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
		assert(false);
	}

	hr = device->CreateRootSignature(
		0,
		blob->GetBufferPointer(),
		blob->GetBufferSize(),
		IID_PPV_ARGS(&result)
	);

	assert(SUCCEEDED(hr));

	return result;

}
