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

	if (pipelines_[pipelineType_].pipeline->IsUseDefaultPipeline()) {
		// default pipeline のときIAが必要になるので
		commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	}
}

void DxObject::PipelineManager::CreatePipelineTable() {

	{
		// rootSignatureDescの初期化
		RootSignatureDesc desc;
		desc.Resize(6, 1);

		// rangeの設定
		D3D12_DESCRIPTOR_RANGE range[1] = {};
		range[0].BaseShaderRegister                = 0;
		range[0].NumDescriptors                    = 1;
		range[0].RangeType                         = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		range[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

		// parameterの設定
		//!< TransformationMatrix
		desc.params[0].ParameterType             = D3D12_ROOT_PARAMETER_TYPE_CBV;
		desc.params[0].ShaderVisibility          = D3D12_SHADER_VISIBILITY_MESH;
		desc.params[0].Descriptor.ShaderRegister = 0;

		desc.params[1].ParameterType             = D3D12_ROOT_PARAMETER_TYPE_CBV;
		desc.params[1].ShaderVisibility          = D3D12_SHADER_VISIBILITY_MESH;
		desc.params[1].Descriptor.ShaderRegister = 1;
		
		//!< Material
		desc.params[2].ParameterType             = D3D12_ROOT_PARAMETER_TYPE_CBV;
		desc.params[2].ShaderVisibility          = D3D12_SHADER_VISIBILITY_PIXEL;
		desc.params[2].Descriptor.ShaderRegister = 0;
		
		//!< Texture
		desc.params[3].ParameterType                       = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		desc.params[3].ShaderVisibility                    = D3D12_SHADER_VISIBILITY_PIXEL;
		desc.params[3].DescriptorTable.pDescriptorRanges   = range;
		desc.params[3].DescriptorTable.NumDescriptorRanges = _countof(range);

		/// mesh shader params

		//!< vertices
		desc.params[4].ParameterType             = D3D12_ROOT_PARAMETER_TYPE_SRV;
		desc.params[4].ShaderVisibility          = D3D12_SHADER_VISIBILITY_MESH;
		desc.params[4].Descriptor.ShaderRegister = 0;
		desc.params[4].Descriptor.RegisterSpace  = 0;

		//!< indices
		desc.params[5].ParameterType             = D3D12_ROOT_PARAMETER_TYPE_SRV;
		desc.params[5].ShaderVisibility          = D3D12_SHADER_VISIBILITY_MESH;
		desc.params[5].Descriptor.ShaderRegister = 1;
		desc.params[5].Descriptor.RegisterSpace  = 0;


		// samplersの設定
		desc.samplers[0].Filter           = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
		desc.samplers[0].AddressU         = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		desc.samplers[0].AddressV         = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		desc.samplers[0].AddressW         = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		desc.samplers[0].ComparisonFunc   = D3D12_COMPARISON_FUNC_NEVER;
		desc.samplers[0].MaxLOD           = D3D12_FLOAT32_MAX;
		desc.samplers[0].ShaderRegister   = 0;
		desc.samplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

		pipelineMenbers_[PipelineType::FLOOR].rootSignature = std::make_unique<RootSignature>(devices_, desc);

		pipelineMenbers_[PipelineType::FLOOR].shaderBlob = std::make_unique<ShaderBlob>();

		auto blob = pipelineMenbers_[PipelineType::FLOOR].shaderBlob.get();
		blob->Create(L"Floor.MS.hlsl", MESH);
		blob->Create(L"Floor.PS.hlsl", PIXEL);

	}

	{
		// rootSignatureDescの初期化
		RootSignatureDesc desc;
		desc.Resize(8, 1);

		// mesh
		//!< vertices
		desc.SetSRV(0, SHADER_MESH, 0);
		
		//!< uniqueVertexIndices
		desc.SetSRV(1, SHADER_MESH, 1);

		//!< primitiveIndices
		desc.SetSRV(2, SHADER_MESH, 2);

		//!< meshlets
		desc.SetSRV(3, SHADER_MESH, 3);

		//!< TransformationMatrix
		desc.SetCBV(4, SHADER_MESH, 0);

		//!< Material
		desc.SetCBV(5, SHADER_PIXEL, 0);

		//!< camera
		desc.SetCBV(6, SHADER_MESH, 1);

		//!< Texture
		desc.SetSRV(7, SHADER_PIXEL, 0);
		desc.SetSampler(0, MODE_WRAP, SHADER_PIXEL, 0);


		pipelineMenbers_[PipelineType::DEFAULT].rootSignature = std::make_unique<DxObject::RootSignature>(devices_, desc);

		pipelineMenbers_[PipelineType::DEFAULT].shaderBlob = std::make_unique<DxObject::ShaderBlob>();
		auto blob = pipelineMenbers_[PipelineType::DEFAULT].shaderBlob.get();
		blob->Create(L"Default.MS.hlsl", MESH);
		blob->Create(L"Default.PS.hlsl", PIXEL);
	}

	{
		// rootSignatureDesc
		RootSignatureDesc desc;
		desc.Resize(2, 0);

		//!< camera
		desc.SetCBV(0, SHADER_VERTEX, 0);

		//!< material
		desc.SetCBV(1, SHADER_PIXEL, 0);

		pipelineMenbers_[PipelineType::PRIMITIVE].rootSignature = std::make_unique<DxObject::RootSignature>(devices_, desc);

		pipelineMenbers_[PipelineType::PRIMITIVE].shaderBlob = std::make_unique<DxObject::ShaderBlob>();
		auto blob = pipelineMenbers_[PipelineType::PRIMITIVE].shaderBlob.get();
		blob->Create(L"Primitive.VS.hlsl", VERTEX);
		blob->Create(L"Primitive.PS.hlsl", PIXEL);
	}

	{
		// rootSignatureDesc
		RootSignatureDesc desc;
		desc.Resize(3, 0);

		//!< camera
		desc.SetCBV(0, SHADER_VERTEX, 0);

		//!< material
		desc.SetCBV(1, SHADER_PIXEL, 0);

		//!< light
		desc.SetCBV(2, SHADER_PIXEL, 1);

		pipelineMenbers_[PipelineType::BLASRENDER].rootSignature = std::make_unique<DxObject::RootSignature>(devices_, desc);

		pipelineMenbers_[PipelineType::BLASRENDER].shaderBlob = std::make_unique<DxObject::ShaderBlob>();
		auto blob = pipelineMenbers_[PipelineType::BLASRENDER].shaderBlob.get();
		blob->Create(L"BlasRender.VS.hlsl", VERTEX);
		blob->Create(L"BlasRender.PS.hlsl", PIXEL);
	}

	{
		// rootSignatureDesc
		RootSignatureDesc desc;
		desc.Resize(3, 1);

		//!< camera2D
		desc.SetCBV(0, SHADER_VERTEX, 0);

		//!< blurParam
		desc.SetCBV(1, SHADER_PIXEL, 0);

		//!< textureBuffer
		desc.SetSRV(2, SHADER_PIXEL, 0);
		desc.SetSampler(0, MODE_WRAP, SHADER_PIXEL, 0);

		pipelineMenbers_[PipelineType::GAUSSIANBLUR].rootSignature = std::make_unique<DxObject::RootSignature>(devices_, desc);

		pipelineMenbers_[PipelineType::GAUSSIANBLUR].shaderBlob = std::make_unique<DxObject::ShaderBlob>();
		auto blob = pipelineMenbers_[PipelineType::GAUSSIANBLUR].shaderBlob.get();
		blob->Create(L"GaussianBlur.VS.hlsl", VERTEX);
		blob->Create(L"GaussianBlur.PS.hlsl", PIXEL);
	}

	{
		// rootSignatureDescの初期化
		RootSignatureDesc desc;
		desc.Resize(7, 0);

		// mesh
		//!< vertices
		desc.SetSRV(0, SHADER_MESH, 0);

		//!< uniqueVertexIndices
		desc.SetSRV(1, SHADER_MESH, 1);

		//!< primitiveIndices
		desc.SetSRV(2, SHADER_MESH, 2);

		//!< meshlets
		desc.SetSRV(3, SHADER_MESH, 3);

		//!< TransformationMatrix
		desc.SetCBV(4, SHADER_MESH, 0);

		//!< Material
		desc.SetCBV(5, SHADER_PIXEL, 0);

		//!< camera
		desc.SetCBV(6, SHADER_MESH, 1);


		pipelineMenbers_[PipelineType::PLAYER].rootSignature = std::make_unique<DxObject::RootSignature>(devices_, desc);

		pipelineMenbers_[PipelineType::PLAYER].shaderBlob = std::make_unique<DxObject::ShaderBlob>();
		auto blob = pipelineMenbers_[PipelineType::PLAYER].shaderBlob.get();
		blob->Create(L"Default.MS.hlsl", MESH);
		blob->Create(L"Player.PS.hlsl", PIXEL);
	}
}
