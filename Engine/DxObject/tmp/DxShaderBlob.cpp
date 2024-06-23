#include "DxShaderBlob.h"

//-----------------------------------------------------------------------------------------
// include
//-----------------------------------------------------------------------------------------
#include <Logger.h>

// DxObject
#include <DxShaderTable.h>

//=========================================================================================
// static variables
//=========================================================================================

DxObject::ShaderTable* DxObject::ShaderBlob::shaderTable_ = nullptr;

////////////////////////////////////////////////////////////////////////////////////////////
// ShaderBlob class methods
////////////////////////////////////////////////////////////////////////////////////////////

void DxObject::ShaderBlob::Create(const std::wstring& fileName, ShaderType type) {

	// get shader blob
	shaderBlob_[type] = shaderTable_->GetShaderBlob(fileName, type);
	assert(shaderBlob_[type] != nullptr);

	// mesh shader pipelineを使う場合, tureに
	if (type == MESH) {
		isUseMeshShaders_ = true;
	}

	Log(std::format("[DxObject.ShaderBlob]: shaderBlob_[{}] << Complete Create", static_cast<int>(type)));
}

void DxObject::ShaderBlob::Term() {
	for (int i = 0; i < ShaderType::kCountOfShaderType; ++i) {
		shaderBlob_[i] = nullptr;
	}

	isUseMeshShaders_ = false;
}