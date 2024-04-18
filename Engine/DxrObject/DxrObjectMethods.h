#pragma once

//-----------------------------------------------------------------------------------------
// include
//-----------------------------------------------------------------------------------------
// directX
#include <d3d12.h>
#include <dxgi1_6.h>
#include "externals/DirectXTex/d3dx12.h"

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
// DxrObjectMethod namespace
////////////////////////////////////////////////////////////////////////////////////////////
namespace DxrObjectMethod {

	ComPtr<ID3D12Resource> CreateBufferResource(
		ID3D12Device5* device,
		uint64_t size,
		D3D12_RESOURCE_FLAGS flag,
		D3D12_RESOURCE_STATES initalizeState,
		const D3D12_HEAP_PROPERTIES& prop
	);

	ComPtr<ID3D12RootSignature> CreateRootSignature(
		ID3D12Device5* device,
		const D3D12_ROOT_SIGNATURE_DESC& desc
	);

	// 仮定義
	static const D3D12_HEAP_PROPERTIES kUploadHeapProps
		= CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);

	static const D3D12_HEAP_PROPERTIES kDefaultHeapProps
		= CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

	void WriteToHostVisibleMemory(ID3D12Resource* resource, const void* pData, size_t dataSize);

	ComPtr<ID3D12Resource> CreateTexture2D(
		ID3D12Device5* device,
		UINT width, UINT height,
		DXGI_FORMAT format,
		D3D12_RESOURCE_FLAGS flags,
		D3D12_RESOURCE_STATES initialState,
		const D3D12_HEAP_PROPERTIES& prop
	);

	UINT Alignment(size_t size, UINT align);

}