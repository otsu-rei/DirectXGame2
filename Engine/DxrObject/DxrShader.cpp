#include "DxrShader.h"

const LPCWSTR DxrObject::Shader::kShaderFileName_ = L"RayTracing.hlsl";
const LPCWSTR DxrObject::Shader::kShaderModel_ = L"lib_6_3";
const LPCWSTR DxrObject::Shader::kDefaultHitGroup_ = L"kDefaultHitGroup";

void DxrObject::Shader::Init() {

	auto hr = DxcCreateInstance(
		CLSID_DxcLibrary, IID_PPV_ARGS(&library_)
	);
	assert(SUCCEEDED(hr));

	hr = DxcCreateInstance(
		CLSID_DxcCompiler, IID_PPV_ARGS(&compiler_)
	);
	assert(SUCCEEDED(hr));

	ComPtr<IDxcBlobEncoding> source;
	hr = library_->CreateBlobFromFile(
		kShaderFileName_,
		nullptr,
		&source
	);

	ComPtr<IDxcOperationResult> result;
	hr = compiler_->Compile(
		source.Get(),
		kShaderFileName_,
		L"",
		kShaderModel_,
		nullptr,
		0,
		nullptr,
		0, 
		nullptr,
		&result
	);

	assert(SUCCEEDED(hr));

	hr = result->GetResult(&shaderBlob_);
	assert(SUCCEEDED(hr));

	// nannka
	D3D12_EXPORT_DESC exports[] = {
		{ L"mainRayGen", nullptr, D3D12_EXPORT_FLAG_NONE },
		{ L"mainMS",     nullptr, D3D12_EXPORT_FLAG_NONE },
		{ L"mainCHS",    nullptr, D3D12_EXPORT_FLAG_NONE },
	};

	D3D12_DXIL_LIBRARY_DESC dxilDesc = {};
	dxilDesc.DXILLibrary = { shaderBlob_->GetBufferPointer(), shaderBlob_->GetBufferSize() };
	dxilDesc.NumExports = _countof(exports);
	dxilDesc.pExports = exports;

	subObjects[0].Type = D3D12_STATE_SUBOBJECT_TYPE_DXIL_LIBRARY;
	subObjects[0].pDesc = &dxilDesc;

	// hitgroup
	D3D12_HIT_GROUP_DESC hitDesc = {};
	hitDesc.Type = D3D12_HIT_GROUP_TYPE_TRIANGLES;
	hitDesc.ClosestHitShaderImport = L"mainCHS";
	hitDesc.HitGroupExport = kDefaultHitGroup_;

	subObjects[1].Type = D3D12_STATE_SUBOBJECT_TYPE_HIT_GROUP;
	subObjects[1].pDesc = &hitDesc;



}

void DxrObject::Shader::Term() {
}
