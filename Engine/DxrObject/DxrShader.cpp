#include "DxrShader.h"

const LPCWSTR kShaderFileName_ = L"RayTracing.hlsl";
const LPCWSTR kShaderModel_ = L"lib_6_3";

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



}

void DxrObject::Shader::Term() {
}
