#include "DxrObjectMethods.h"

#include <Logger.h>
#include <cassert>

#include "DxrCommon.h"

ComPtr<ID3D12Resource> DxrObjectMethod::CreateBufferResource(
	ID3D12Device5* device,
	uint64_t size,
	D3D12_RESOURCE_FLAGS flag,
	D3D12_RESOURCE_STATES initalizeState,
	const D3D12_HEAP_PROPERTIES& prop) {

	D3D12_RESOURCE_DESC resDesc = {};
	resDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resDesc.Alignment = 0;
	resDesc.Width = size;
	resDesc.Height = 1;
	resDesc.DepthOrArraySize = 1;
	resDesc.MipLevels = 1;
	resDesc.Format = DXGI_FORMAT_UNKNOWN;
	resDesc.SampleDesc = { 1, 0 };
	resDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	resDesc.Flags = flag;
	
	ComPtr<ID3D12Resource> buffer = nullptr;
	auto hr = device->CreateCommittedResource(
		&prop,
		D3D12_HEAP_FLAG_NONE,
		&resDesc,
		initalizeState,
		nullptr,
		IID_PPV_ARGS(&buffer)
	);

	assert(SUCCEEDED(hr));

	return buffer;
}

ComPtr<ID3D12Resource> DxrObjectMethod::CreateBufferResource(
	ID3D12Device5* device,
	uint64_t size,
	D3D12_RESOURCE_FLAGS flag,
	D3D12_RESOURCE_STATES initalizeState,
	D3D12_HEAP_TYPE heapType,
	const wchar_t* name) {
	
	D3D12_HEAP_PROPERTIES prop = {};
	
	if (heapType == D3D12_HEAP_TYPE_DEFAULT) {
		prop = kDefaultHeapProps;

	} else if (heapType == D3D12_HEAP_TYPE_UPLOAD) {
		prop = kUploadHeapProps;
	}

	ComPtr<ID3D12Resource> result;
	D3D12_RESOURCE_DESC desc = {};
	desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	desc.Alignment = 0;
	desc.Width = size;
	desc.Height = 1;
	desc.DepthOrArraySize = 1;
	desc.MipLevels = 1;
	desc.Format = DXGI_FORMAT_UNKNOWN;
	desc.SampleDesc = { 1, 0 };
	desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	desc.Flags = flag;

	auto hr = device->CreateCommittedResource(
		&prop,
		D3D12_HEAP_FLAG_NONE,
		&desc,
		initalizeState,
		nullptr,
		IID_PPV_ARGS(&result)
	);

	assert(SUCCEEDED(hr));

	if (result != nullptr && name != nullptr) {
		result->SetName(name);
	}

	return result;

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

void DxrObjectMethod::WriteToHostVisibleMemory(ID3D12Resource* resource, const void* pData, size_t dataSize) {
	D3D12_RANGE readRange = { 0, dataSize }; // マップ時の読み取り範囲を指定
	void* mappedData;
	auto hr = resource->Map(0, &readRange, &mappedData);
	if (SUCCEEDED(hr)) {
		// データをバッファに書き込む
		memcpy(mappedData, pData, dataSize);

		// バッファのアンマップ
		resource->Unmap(0, nullptr);
	}
}

ComPtr<ID3D12Resource> DxrObjectMethod::CreateTexture2D(
	ID3D12Device5* device,
	UINT width, UINT height,
	DXGI_FORMAT format,
	D3D12_RESOURCE_FLAGS flags,
	D3D12_RESOURCE_STATES initialState,
	const D3D12_HEAP_PROPERTIES& prop) {

	ComPtr<ID3D12Resource> result;
	
	D3D12_RESOURCE_DESC desc = {};
	desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	desc.Alignment = 0;
	desc.Width = width;
	desc.Height = height;
	desc.DepthOrArraySize = 1;
	desc.MipLevels = 1;
	desc.Format = format;
	desc.SampleDesc = { 1, 0 };
	desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	desc.Flags = flags;

	auto hr = device->CreateCommittedResource(
		&prop,
		D3D12_HEAP_FLAG_NONE,
		&desc,
		initialState,
		nullptr,
		IID_PPV_ARGS(&result)
	);

	assert(SUCCEEDED(hr));

	return result;

}

UINT DxrObjectMethod::Alignment(size_t size, UINT align) {
	return UINT(size + align - 1) & ~(align - 1);
}
