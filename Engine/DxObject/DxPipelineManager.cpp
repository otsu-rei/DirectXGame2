#include "DxPipelineManager.h"

//-----------------------------------------------------------------------------------------
// include
//-----------------------------------------------------------------------------------------
// DxObject
#include <DxDevices.h>
#include <DxCommand.h>

////////////////////////////////////////////////////////////////////////////////////////////
// PipelineManager methods
////////////////////////////////////////////////////////////////////////////////////////////

DxObject::PipelineManager::PipelineManager(
	Devices* devices, Command* command, BlendState* blendState,
	int32_t clientWidth, int32_t clientHeight) {

	Init(devices, command, blendState, clientWidth, clientHeight);
}

void DxObject::PipelineManager::Init(
	Devices* devices, Command* command, BlendState* blendState,
	int32_t clientWidth, int32_t clientHeight) {

	// Adapterを保存
	devices_ = devices;
	command_ = command;

	// blendstate用に保存
	blendState_ = blendState;

	/// viewportsの初期化 ///
	// viewportの設定
	{
		viewport_.Width    = static_cast<float>(clientWidth);
		viewport_.Height   = static_cast<float>(clientHeight);
		viewport_.TopLeftX = 0;
		viewport_.TopLeftY = 0;
		viewport_.MinDepth = 0.0f;
		viewport_.MaxDepth = 1.0f;
	}

	// シザー矩形の設定
	{
		scissorRect_.left   = 0;
		scissorRect_.right  = clientWidth;
		scissorRect_.top    = 0;
		scissorRect_.bottom = clientHeight;
	}

	CreatePipelineTable();
	
}

void DxObject::PipelineManager::Term() {
	for (int i = 0; i < PipelineType::kCountOfPipeline; ++i) {
		pipelineMenbers_[i].Reset();
	}

	/*pipelineState_.reset();*/

	for (auto& it : pipelines_) {
		it.pipeline.reset();
	}
}

void DxObject::PipelineManager::CreatePipeline() {

	if ((pipelines_[pipelineType_].settingBlendMode_ != blendMode_) || (pipelines_[pipelineType_].pipeline == nullptr)) {
		pipelines_[pipelineType_].pipeline = std::make_unique<PipelineState>(
			devices_,
			pipelineMenbers_[pipelineType_].shaderBlob.get(), pipelineMenbers_[pipelineType_].rootSignature.get(),
			blendState_->operator[](blendMode_)
		);

		pipelines_[pipelineType_].settingBlendMode_ = blendMode_;
	}
}

void DxObject::PipelineManager::SetPipeline() {
	assert(pipelines_[pipelineType_].pipeline != nullptr);

	// commandListの取り出し
	ID3D12GraphicsCommandList* commandList = command_->GetCommandList();

	commandList->RSSetViewports(1, &viewport_);
	commandList->RSSetScissorRects(1, &scissorRect_);

	commandList->SetGraphicsRootSignature(pipelineMenbers_[pipelineType_].rootSignature->GetRootSignature());
	commandList->SetPipelineState(pipelines_[pipelineType_].pipeline->GetPipelineState());

	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void DxObject::PipelineManager::CreatePipelineTable() {

	{
		// rootSignatureDescsの初期化
		DxObject::RootSignatureDescs desc(3, 1);

		// rangeの設定
		D3D12_DESCRIPTOR_RANGE range[1] = {};
		range[0].BaseShaderRegister                = 0;
		range[0].NumDescriptors                    = 1;
		range[0].RangeType                         = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		range[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

		// parameterの設定
		//!< FloorForGPU
		desc.param[0].ParameterType             = D3D12_ROOT_PARAMETER_TYPE_CBV;
		desc.param[0].ShaderVisibility          = D3D12_SHADER_VISIBILITY_VERTEX;
		desc.param[0].Descriptor.ShaderRegister = 0;
		
		//!< Material
		desc.param[1].ParameterType             = D3D12_ROOT_PARAMETER_TYPE_CBV;
		desc.param[1].ShaderVisibility          = D3D12_SHADER_VISIBILITY_PIXEL;
		desc.param[1].Descriptor.ShaderRegister = 0;
		
		//!< Texture
		desc.param[2].ParameterType                       = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		desc.param[2].ShaderVisibility                    = D3D12_SHADER_VISIBILITY_PIXEL;
		desc.param[2].DescriptorTable.pDescriptorRanges   = range;
		desc.param[2].DescriptorTable.NumDescriptorRanges = _countof(range);

		// samplerの設定
		desc.sampler[0].Filter           = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
		desc.sampler[0].AddressU         = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		desc.sampler[0].AddressV         = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		desc.sampler[0].AddressW         = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		desc.sampler[0].ComparisonFunc   = D3D12_COMPARISON_FUNC_NEVER;
		desc.sampler[0].MaxLOD           = D3D12_FLOAT32_MAX;
		desc.sampler[0].ShaderRegister   = 0;
		desc.sampler[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

		pipelineMenbers_[PipelineType::FLOOR].rootSignature = std::make_unique<DxObject::RootSignature>(devices_, desc);

		pipelineMenbers_[PipelineType::FLOOR].shaderBlob = std::make_unique<DxObject::ShaderBlob>(
			L"Floor.VS.hlsl", L"Floor.GS.hlsl", L"Floor.PS.hlsl"
		);

	}

	{
		// rootSignatureDescsの初期化
		DxObject::RootSignatureDescs desc(3, 1);

		// rangeの設定
		D3D12_DESCRIPTOR_RANGE range[1] = {};
		range[0].BaseShaderRegister                = 0;
		range[0].NumDescriptors                    = 1;
		range[0].RangeType                         = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		range[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

		D3D12_DESCRIPTOR_RANGE rangeInstanceing[1] = {};
		rangeInstanceing[0].BaseShaderRegister                = 0;
		rangeInstanceing[0].NumDescriptors                    = 1;
		rangeInstanceing[0].RangeType                         = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		rangeInstanceing[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

		// parameterの設定
		desc.param[0].ParameterType             = D3D12_ROOT_PARAMETER_TYPE_CBV;
		desc.param[0].ShaderVisibility          = D3D12_SHADER_VISIBILITY_PIXEL;
		desc.param[0].Descriptor.ShaderRegister = 0;

		desc.param[1].ParameterType                       = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		desc.param[1].ShaderVisibility                    = D3D12_SHADER_VISIBILITY_VERTEX;
		desc.param[1].DescriptorTable.pDescriptorRanges   = rangeInstanceing;
		desc.param[1].DescriptorTable.NumDescriptorRanges = _countof(rangeInstanceing);

		desc.param[2].ParameterType                       = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		desc.param[2].ShaderVisibility                    = D3D12_SHADER_VISIBILITY_PIXEL;
		desc.param[2].DescriptorTable.pDescriptorRanges   = range;
		desc.param[2].DescriptorTable.NumDescriptorRanges = _countof(range);

		// samplerの設定
		desc.sampler[0].Filter           = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
		desc.sampler[0].AddressU         = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		desc.sampler[0].AddressV         = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		desc.sampler[0].AddressW         = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		desc.sampler[0].ComparisonFunc   = D3D12_COMPARISON_FUNC_NEVER;
		desc.sampler[0].MaxLOD           = D3D12_FLOAT32_MAX;
		desc.sampler[0].ShaderRegister   = 0;
		desc.sampler[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

		pipelineMenbers_[PipelineType::PARTICLE].rootSignature = std::make_unique<DxObject::RootSignature>(devices_, desc);

		pipelineMenbers_[PipelineType::PARTICLE].shaderBlob = std::make_unique<DxObject::ShaderBlob>(
			L"Particle.VS.hlsl", L"Particle.GS.hlsl", L"Particle.PS.hlsl"
		);
	}

	// sphere
	{
		// rootSignatureDescsの初期化
		DxObject::RootSignatureDescs desc(1, 0);

		// parameterの設定
		//!< Matrix
		desc.param[0].ParameterType             = D3D12_ROOT_PARAMETER_TYPE_CBV;
		desc.param[0].ShaderVisibility          = D3D12_SHADER_VISIBILITY_VERTEX;
		desc.param[0].Descriptor.ShaderRegister = 0;

		pipelineMenbers_[PipelineType::SPHERE].rootSignature = std::make_unique<DxObject::RootSignature>(devices_, desc);

		pipelineMenbers_[PipelineType::SPHERE].shaderBlob = std::make_unique<DxObject::ShaderBlob>(
			L"Sphere.VS.hlsl", L"Sphere.PS.hlsl"
		);
	}

}
