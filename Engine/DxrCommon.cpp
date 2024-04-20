#include "DxrCommon.h"

#include <Vector3.h>
#include "DxrObjectMethods.h"
#include <array>

#include "Logger.h"

//-----------------------------------------------------------------------------------------
// using
//-----------------------------------------------------------------------------------------
using namespace DxrObject;
using namespace DxrObjectMethod;

const LPCWSTR DxrCommon::kShaderModel_ = L"lib_6_3";
const LPCWSTR DxrCommon::kShaderFileName_ = L"./Resources/DxrHlsl/RayTracing.hlsl";

const LPCWSTR DxrCommon::kRayGenerationFunctionName_ = L"mainRayGen";
const LPCWSTR DxrCommon::kClosestHitFunctionName_ = L"mainCHS";
const LPCWSTR DxrCommon::kMissFunctionName_ = L"mainMS";

const LPCWSTR DxrCommon::kHitGroup_ = L"hitGroup";

////////////////////////////////////////////////////////////////////////////////////////////
// DxrCommon class methods
////////////////////////////////////////////////////////////////////////////////////////////

void DxrCommon::Init(WinApp* winApp, int32_t clientWidth, int32_t clientHeight) {
	devices_         = std::make_unique<Devices>();
	command_         = std::make_unique<Command>(devices_.get());
	descriptorHeaps_ = std::make_unique<DescriptorHeaps>(devices_.get());
	swapChain_       = std::make_unique<SwapChain>(
		devices_.get(), command_.get(), descriptorHeaps_.get(),
		winApp, clientWidth, clientHeight
	);
	fence_           = std::make_unique<Fence>(devices_.get());

	/*CreateBLAS();
	CreateTLAS();
	CreateRootSignatureGlobal();
	CreateShaderBlob();
	CreateStateObject();
	CreateResultBuffer(clientWidth, clientHeight);
	CreateShaderTable(clientWidth, clientHeight);*/

	CreateObject();
	CreateSceneBLAS();
	CreateSceneTLAS();
	CreateGlobalRootSignature();
	CreateLoaclRootSignatureRayGen();
	CreateLoaclRootSignatureColosestHit();
	CreateShaderBlob();
	CreateStateObject();
	CreateResultBuffer(clientWidth, clientHeight);
	CreateShaderTable(clientWidth, clientHeight);
	
	Sent();
}

void DxrCommon::Term() {


	fence_.reset();
	swapChain_.reset();
	descriptorHeaps_.reset();
	command_.reset();
	devices_.reset();


}

void DxrCommon::BeginFrame() {
	// コマンドリストの取得
	auto commandList = command_->GetCommandList();

	// 書き込みバックバッファのインデックスを取得
	backBufferIndex_ = swapChain_->GetSwapChain()->GetCurrentBackBufferIndex();

	commandList->ResourceBarrier(
		1,
		swapChain_->GetTransitionBarrier(backBufferIndex_, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET)
	);

	D3D12_CPU_DESCRIPTOR_HANDLE handle_RTV = swapChain_->GetHandleCPU_RTV(backBufferIndex_);

	commandList->OMSetRenderTargets(
		1,
		&handle_RTV,
		false,
		nullptr
	);

	// 画面のクリア
	float clearColor[] = { 0.6f, 0.8f, 0.4f, 1.0f };
	commandList->ClearRenderTargetView(
		handle_RTV,
		clearColor,
		0, nullptr
	);
}

void DxrCommon::EndFrame() {
	command_->GetCommandList()->ResourceBarrier(
		1,
		swapChain_->GetTransitionBarrier(backBufferIndex_, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT)
	);

	command_->Close();

	swapChain_->Present(1, 0);

	// GPUにシグナルを送る
	fence_->AddFenceValue();

	command_->Signal(fence_.get());

	fence_->WaitGPU();

	command_->Reset();
}

void DxrCommon::DxrRender() {

	backBufferIndex_ = swapChain_->GetSwapChain()->GetCurrentBackBufferIndex();

	auto commandList = command_->GetCommandList();

	ID3D12DescriptorHeap* descriptorHeaps[] = {
		descriptorHeaps_->GetDescriptorHeap(SRV)
	};
	commandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);
	commandList->SetComputeRootSignature(rootSignatureGlobal_.Get());

	commandList->SetComputeRootDescriptorTable(0, tlasDescriptor_.handleGPU);

	// レイトレーシング結果バッファを UAV 状態へ.
	auto barrierToUAV = CD3DX12_RESOURCE_BARRIER::Transition(
		outputResource_.Get(),
		D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS
	);
	commandList->ResourceBarrier(1, &barrierToUAV);

	// レイトレーシングを開始.
	commandList->SetPipelineState1(stateObject_.Get());
	commandList->DispatchRays(&dispatchRayDesc_);

	// レイトレーシング結果をバックバッファへコピーする.
	// バリアを設定し各リソースの状態を遷移させる.
	D3D12_RESOURCE_BARRIER barriers[] = {
		CD3DX12_RESOURCE_BARRIER::Transition(
			outputResource_.Get(),
			D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE),
		CD3DX12_RESOURCE_BARRIER::Transition(
			swapChain_->GetResource(backBufferIndex_),
			D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_COPY_DEST),
	};
	commandList->ResourceBarrier(_countof(barriers), barriers);
	commandList->CopyResource(swapChain_->GetResource(backBufferIndex_), outputResource_.Get());

	// Present 可能なようにバリアをセット.
	auto barrierToPresent = CD3DX12_RESOURCE_BARRIER::Transition(
		swapChain_->GetResource(backBufferIndex_),
		D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PRESENT
	);
	commandList->ResourceBarrier(1, &barrierToPresent);

	command_->Close();

	swapChain_->Present(1, 0);

	// GPUにシグナルを送る
	fence_->AddFenceValue();

	command_->Signal(fence_.get());

	fence_->WaitGPU();

	command_->Reset();
}

void DxrCommon::Sent() {
	command_->Close();

	// GPUにシグナルを送る
	fence_->AddFenceValue();

	command_->Signal(fence_.get());

	fence_->WaitGPU();

	command_->Reset();
}

DxrCommon* DxrCommon::GetInstance() {
	static DxrCommon instance;
	return &instance;
}

void DxrCommon::CreateBLAS() {
	auto device = devices_->GetDevice();
	auto command = command_->GetCommandList();

	Vector3f tri[3] = {
		{-0.5f, -0.5f, 0.0f},
		{0.5f, -0.5f, 0.0f},
		{0.0f, 0.75f, 0.0f}
	};

	auto vbSize = sizeof(tri);
	vertexBuffer_ = CreateBufferResource(
		devices_->GetDevice(),
		vbSize,
		D3D12_RESOURCE_FLAG_NONE,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		kUploadHeapProps
	);

	// resourceをマッピング
	WriteToHostVisibleMemory(vertexBuffer_.Get(), tri, vbSize);
	vertexBuffer_->SetName(L"vertexBuffer_");

	// BLAS
	D3D12_RAYTRACING_GEOMETRY_DESC geomDesc{};
	geomDesc.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
	geomDesc.Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE;
	geomDesc.Triangles.VertexBuffer.StartAddress = vertexBuffer_->GetGPUVirtualAddress();
	geomDesc.Triangles.VertexBuffer.StrideInBytes = sizeof(Vector3f);
	geomDesc.Triangles.VertexFormat = DXGI_FORMAT_R32G32B32_FLOAT;
	geomDesc.Triangles.VertexCount = _countof(tri);

	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC buildASDesc{};
	auto& inputs = buildASDesc.Inputs;
	inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
	inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
	inputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_NONE;
	inputs.NumDescs = 1;
	inputs.pGeometryDescs = &geomDesc;

	// 必要なメモリ量を求める.
	D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO blasPrebuild{};
	device->GetRaytracingAccelerationStructurePrebuildInfo(
		&inputs, &blasPrebuild
	);

	// スクラッチバッファを確保.
	ComPtr<ID3D12Resource> blasScratch;
	blasScratch = CreateBufferResource(
		device,
		blasPrebuild.ScratchDataSizeInBytes,
		D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
		kDefaultHeapProps
	);

	blas_ = CreateBufferResource(
		device,
		blasPrebuild.ResultDataMaxSizeInBytes,
		D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
		D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE,
		kDefaultHeapProps
	);
	blas_->SetName(L"test");

	buildASDesc.ScratchAccelerationStructureData = blasScratch->GetGPUVirtualAddress();
	buildASDesc.DestAccelerationStructureData = blas_->GetGPUVirtualAddress();

	command->BuildRaytracingAccelerationStructure(
		&buildASDesc, 0, nullptr
	);

	// リソースバリアの設定.
	D3D12_RESOURCE_BARRIER uavBarrier = {};
	uavBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
	uavBarrier.UAV.pResource = blas_.Get();
	command->ResourceBarrier(1, &uavBarrier);

	// BLAS 構築.
	Sent();
}

void DxrCommon::CreateTLAS() {
	auto device = devices_->GetDevice();
	auto command = command_->GetCommandList();

	D3D12_RAYTRACING_INSTANCE_DESC instanceDesc = {};
	instanceDesc.InstanceID = 0;
	instanceDesc.InstanceMask = 0xFF;
	instanceDesc.InstanceContributionToHitGroupIndex = 0;
	instanceDesc.Flags = D3D12_RAYTRACING_INSTANCE_FLAG_NONE;
	instanceDesc.AccelerationStructure = blas_->GetGPUVirtualAddress();
	Matrix4x4 mat = Matrix4x4::MakeIdentity();
	mat = Matrix::Transpose(mat);
	memcpy(&instanceDesc.Transform, &mat, sizeof(instanceDesc.Transform));

	// インスタンスの情報を記録したバッファを準備する.
	ComPtr<ID3D12Resource> instanceDescBuffer;
	instanceDescBuffer = CreateBufferResource(
		device,
		sizeof(instanceDesc),
		D3D12_RESOURCE_FLAG_NONE,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		kUploadHeapProps
	);

	WriteToHostVisibleMemory(
		instanceDescBuffer.Get(),
		&instanceDesc, sizeof(instanceDesc)
	);

	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC buildASDesc{};
	auto& inputs = buildASDesc.Inputs;
	inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;
	inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
	inputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_NONE;
	inputs.NumDescs = 1;

	// 必要なメモリ量を求める.
	D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO tlasPrebuild{};
	device->GetRaytracingAccelerationStructurePrebuildInfo(
		&inputs, &tlasPrebuild
	);

	// スクラッチバッファを確保.
	ComPtr<ID3D12Resource> tlasScratch;
	tlasScratch = CreateBufferResource(
		device,
		tlasPrebuild.ScratchDataSizeInBytes,
		D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
		kDefaultHeapProps
	);

	// TLAS 用メモリ(バッファ)を確保.
	// リソースステートは D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE
	tlas_ = CreateBufferResource(
		device,
		tlasPrebuild.ResultDataMaxSizeInBytes,
		D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
		D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE,
		kDefaultHeapProps
	);
	tlas_->SetName(L"tlas_");

	// Acceleration Structure 構築.
	buildASDesc.Inputs.InstanceDescs = instanceDescBuffer->GetGPUVirtualAddress();
	buildASDesc.ScratchAccelerationStructureData = tlasScratch->GetGPUVirtualAddress();
	buildASDesc.DestAccelerationStructureData = tlas_->GetGPUVirtualAddress();

	// コマンドリストに積んで実行する.
	command->BuildRaytracingAccelerationStructure(
		&buildASDesc, 0, nullptr /* pPostBuildInfoDescs */
	);

	// リソースバリアの設定.
	D3D12_RESOURCE_BARRIER uavBarrier{};
	uavBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
	uavBarrier.UAV.pResource = tlas_.Get();
	command->ResourceBarrier(1, &uavBarrier);

	// TLAS 構築.
	Sent();

	// ディスクリプタの準備.
	tlasDescriptor_ = descriptorHeaps_->GetCurrentDescriptor(SRV);

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_RAYTRACING_ACCELERATION_STRUCTURE;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.RaytracingAccelerationStructure.Location = tlas_->GetGPUVirtualAddress();
	device->CreateShaderResourceView(nullptr, &srvDesc, tlasDescriptor_.handleCPU);
}

void DxrCommon::CreateRootSignatureGlobal() {
	auto device = devices_->GetDevice();

	std::array<D3D12_ROOT_PARAMETER, 2> rootParams{};

	// TLAS を t0 レジスタに割り当てて使用する設定.
	D3D12_DESCRIPTOR_RANGE descRangeTLAS{};
	descRangeTLAS.BaseShaderRegister = 0;
	descRangeTLAS.NumDescriptors = 1;
	descRangeTLAS.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;

	// 出力バッファ(UAV) を u0 レジスタに割り当てて使用する設定.
	D3D12_DESCRIPTOR_RANGE descRangeOutput{};
	descRangeOutput.BaseShaderRegister = 0;
	descRangeOutput.NumDescriptors = 1;
	descRangeOutput.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;

	rootParams[0] = D3D12_ROOT_PARAMETER{};
	rootParams[1] = D3D12_ROOT_PARAMETER{};

	rootParams[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParams[0].DescriptorTable.NumDescriptorRanges = 1;
	rootParams[0].DescriptorTable.pDescriptorRanges = &descRangeTLAS;

	rootParams[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParams[1].DescriptorTable.NumDescriptorRanges = 1;
	rootParams[1].DescriptorTable.pDescriptorRanges = &descRangeOutput;

	D3D12_ROOT_SIGNATURE_DESC rootSigDesc{};
	rootSigDesc.NumParameters = UINT(rootParams.size());
	rootSigDesc.pParameters = rootParams.data();

	HRESULT hr;
	ComPtr<ID3DBlob> blob, errBlob;
	hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1_0, &blob, &errBlob);
	if (FAILED(hr)) {
		assert(false);
	}

	hr = device->CreateRootSignature(
		0, blob->GetBufferPointer(), blob->GetBufferSize(),
		IID_PPV_ARGS(rootSignatureGlobal_.ReleaseAndGetAddressOf())
	);
	if (FAILED(hr)) {
		assert(false);
	}
	rootSignatureGlobal_->SetName(L"rootSignatureGlobal_");
}

void DxrCommon::CreateShaderBlob() {
	ComPtr<IDxcLibrary> library;
	ComPtr<IDxcCompiler> compiler;

	auto hr = DxcCreateInstance(
		CLSID_DxcLibrary, IID_PPV_ARGS(&library)
	);
	assert(SUCCEEDED(hr));

	hr = DxcCreateInstance(
		CLSID_DxcCompiler, IID_PPV_ARGS(&compiler)
	);
	assert(SUCCEEDED(hr));

	ComPtr<IDxcBlobEncoding> source;
	hr = library->CreateBlobFromFile(
		kShaderFileName_, nullptr, &source
	);
	assert(SUCCEEDED(hr));

	ComPtr<IDxcOperationResult> result;
	hr = compiler->Compile(
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

	ComPtr<IDxcBlobEncoding> errorBlob;
	hr = result->GetErrorBuffer(&errorBlob);

	if (SUCCEEDED(hr)) {
		if (errorBlob != nullptr && errorBlob->GetBufferSize() != 0) {
			// エラーメッセージを表示するか、適切な処理を行う
			Log(reinterpret_cast<const char*>(errorBlob->GetBufferPointer()));
			assert(false);
		}
	}

	hr = result->GetResult(&shaderBlob_);
	assert(SUCCEEDED(hr));

}
	

void DxrCommon::CreateStateObject() {
	auto device = devices_->GetDevice();

	CD3DX12_STATE_OBJECT_DESC stateObjectDesc{ D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE };

	auto dxilLibrarySubobject
		= stateObjectDesc.CreateSubobject<CD3DX12_DXIL_LIBRARY_SUBOBJECT>();

	D3D12_SHADER_BYTECODE libdxil = {};
	libdxil.BytecodeLength = shaderBlob_->GetBufferSize();
	libdxil.pShaderBytecode = shaderBlob_->GetBufferPointer();

	LPCWSTR exports[] = {
		kRayGenerationFunctionName_, kClosestHitFunctionName_, kMissFunctionName_
	};

	dxilLibrarySubobject->SetDXILLibrary(&libdxil);
	dxilLibrarySubobject->DefineExports(exports);

	auto hitGroupSubobject
		= stateObjectDesc.CreateSubobject<CD3DX12_HIT_GROUP_SUBOBJECT>();

	hitGroupSubobject->SetClosestHitShaderImport(kClosestHitFunctionName_);
	hitGroupSubobject->SetHitGroupExport(kHitGroup_);
	hitGroupSubobject->SetHitGroupType(D3D12_HIT_GROUP_TYPE_TRIANGLES);

	auto shaderConfig
		= stateObjectDesc.CreateSubobject<CD3DX12_RAYTRACING_SHADER_CONFIG_SUBOBJECT>();

	uint32_t payloadSize = static_cast<uint32_t>(sizeof(Vector3f)) + static_cast<uint32_t>(sizeof(uint32_t));
	uint32_t attributeSize = static_cast<uint32_t>(sizeof(Vector2f));

	shaderConfig->Config(payloadSize, attributeSize);

	// localがないので飛ばしとく error出たらここが原因かも
	auto localRootSignature
		= stateObjectDesc.CreateSubobject<CD3DX12_LOCAL_ROOT_SIGNATURE_SUBOBJECT>();

	localRootSignature->SetRootSignature(modelRootSignature_.Get());

	auto assoc
		= stateObjectDesc.CreateSubobject<CD3DX12_SUBOBJECT_TO_EXPORTS_ASSOCIATION_SUBOBJECT>();

	assoc->AddExport(kHitGroup_);
	assoc->SetSubobjectToAssociate(localRootSignature->operator const D3D12_STATE_SUBOBJECT &());

	auto rayGen
		= stateObjectDesc.CreateSubobject<CD3DX12_LOCAL_ROOT_SIGNATURE_SUBOBJECT>();

	rayGen->SetRootSignature(rayGenRootSignature_.Get());

	auto rayGenSubObject
		= stateObjectDesc.CreateSubobject<CD3DX12_SUBOBJECT_TO_EXPORTS_ASSOCIATION_SUBOBJECT>();

	rayGenSubObject->AddExport(kRayGenerationFunctionName_);
	rayGenSubObject->SetSubobjectToAssociate(rayGen->operator const D3D12_STATE_SUBOBJECT &());


	auto grobalRootSignatureSubobject
		= stateObjectDesc.CreateSubobject<CD3DX12_GLOBAL_ROOT_SIGNATURE_SUBOBJECT>();

	grobalRootSignatureSubobject->SetRootSignature(rootSignatureGlobal_.Get());

	auto pipelineConfigSubobject
		= stateObjectDesc.CreateSubobject<CD3DX12_RAYTRACING_PIPELINE_CONFIG_SUBOBJECT>();

	uint32_t maxRecursionDepth = 16; //!< Max31

	pipelineConfigSubobject->Config(maxRecursionDepth);

	auto hr = device->CreateStateObject(stateObjectDesc, IID_PPV_ARGS(&stateObject_));
	assert(SUCCEEDED(hr));
}

void DxrCommon::CreateResultBuffer(int32_t clientWidth, int32_t clientHeight) {
	auto device = devices_->GetDevice();

	outputResource_ = CreateTexture2D(
		device,
		clientWidth, clientHeight,
		DXGI_FORMAT_R8G8B8A8_UNORM,
		D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
		D3D12_RESOURCE_STATE_COPY_SOURCE,
		kDefaultHeapProps
	);

	// ディスクリプタの準備.
	outputDescriptor_ = descriptorHeaps_->GetCurrentDescriptor(SRV);

	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
	uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;

	device->CreateUnorderedAccessView(outputResource_.Get(), nullptr, &uavDesc, outputDescriptor_.handleCPU);
}

void DxrCommon::CreateShaderTable(int32_t clientWidth, int32_t clientHeight) {
	const auto ShaderRecordAlignment = D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT;
	// RayGeneration シェーダーでは、 Shader Identifier と
	// ローカルルートシグネチャによる u0 のディスクリプタを使用.
	UINT raygenRecordSize = 0;
	raygenRecordSize += D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;
	raygenRecordSize += sizeof(D3D12_GPU_DESCRIPTOR_HANDLE);
	raygenRecordSize = Alignment(raygenRecordSize, ShaderRecordAlignment);

	// ヒットグループでは、 Shader Identifier の他に
	// ローカルルートシグネチャによる VB/IB のディスクリプタを使用.
	UINT hitgroupRecordSize = 0;
	hitgroupRecordSize += D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;
	hitgroupRecordSize += sizeof(D3D12_GPU_DESCRIPTOR_HANDLE);
	hitgroupRecordSize += sizeof(D3D12_GPU_DESCRIPTOR_HANDLE);
	hitgroupRecordSize = Alignment(hitgroupRecordSize, ShaderRecordAlignment);

	// Missシェーダーではローカルルートシグネチャ未使用.
	UINT missRecordSize = 0;
	missRecordSize += D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;
	missRecordSize = Alignment(missRecordSize, ShaderRecordAlignment);

	// 使用する各シェーダーの個数より、シェーダーテーブルのサイズを求める.
	//  RayGen : 1
	//  Miss : 1
	//  HitGroup: 1 (オブジェクトは3つ描画するがヒットグループは2つで処理する).
	UINT hitgroupCount = 1;
	UINT raygenSize = 1 * raygenRecordSize;
	UINT missSize = 1 * missRecordSize;
	UINT hitGroupSize = hitgroupCount * hitgroupRecordSize;

	// 各テーブルの開始位置にアライメント制約があるので調整する.
	auto tableAlign = D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT;
	auto raygenRegion = Alignment(raygenSize, tableAlign);
	auto missRegion = Alignment(missSize, tableAlign);
	auto hitgroupRegion = Alignment(hitGroupSize, tableAlign);

	// シェーダーテーブル確保.
	auto tableSize = raygenRegion + missRegion + hitgroupRegion;
	shaderTable_ = CreateBuffer(
		tableSize, nullptr,
		D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_FLAG_NONE,
		L"ShaderTable");

	ComPtr<ID3D12StateObjectProperties> rtsoProps;
	stateObject_.As(&rtsoProps);

	// 各シェーダーレコードを書き込んでいく.
	void* mapped = nullptr;
	shaderTable_->Map(0, nullptr, &mapped);
	uint8_t* pStart = static_cast<uint8_t*>(mapped);

	// RayGeneration 用のシェーダーレコードを書き込み.
	auto rgsStart = pStart;
	{
		uint8_t* p = rgsStart;
		auto id = rtsoProps->GetShaderIdentifier(L"mainRayGen");
		if (id == nullptr) {
			assert(false);
		}
		p += WriteShaderIdentifier(p, id);

		// ローカルルートシグネチャで u0 (出力先) を設定しているため
		// 対応するディスクリプタを書き込む.
		p += WriteGPUDescriptor(p, outputDescriptor_);
	}

	// Miss Shader 用のシェーダーレコードを書き込み.
	auto missStart = pStart + raygenRegion;
	{
		uint8_t* p = missStart;
		auto id = rtsoProps->GetShaderIdentifier(L"mainMS");
		if (id == nullptr) {
			assert(false);
		}
		p += WriteShaderIdentifier(p, id);

		// ローカルルートシグネチャ使用時には他のデータを書き込む.
	}

	// Hit Group 用のシェーダーレコードを書き込み.
	auto hitgroupStart = pStart + raygenRegion + missRegion;
	{
		uint8_t* pRecord = hitgroupStart;

		// plane に対応するシェーダーレコードを書き込む.
		pRecord = WriteShaderRecord(pRecord, plane_, hitgroupRecordSize);

		// cube に対応するシェーダーレコードを書き込む.
		pRecord = WriteShaderRecord(pRecord, cube_, hitgroupRecordSize);
	}

	shaderTable_->Unmap(0, nullptr);

	// DispatchRays のために情報をセットしておく.
	auto startAddress = shaderTable_->GetGPUVirtualAddress();

	auto& shaderRecordRG = dispatchRayDesc_.RayGenerationShaderRecord;
	shaderRecordRG.StartAddress = startAddress;
	shaderRecordRG.SizeInBytes = raygenSize;
	startAddress += raygenRegion;

	auto& shaderRecordMS = dispatchRayDesc_.MissShaderTable;
	shaderRecordMS.StartAddress = startAddress;
	shaderRecordMS.SizeInBytes = missSize;
	shaderRecordMS.StrideInBytes = missRecordSize;
	startAddress += missRegion;

	auto& shaderRecordHG = dispatchRayDesc_.HitGroupTable;
	shaderRecordHG.StartAddress = startAddress;
	shaderRecordHG.SizeInBytes = hitGroupSize;
	shaderRecordHG.StrideInBytes = hitgroupRecordSize;
	startAddress += hitgroupRegion;

	dispatchRayDesc_.Width = clientWidth;
	dispatchRayDesc_.Height = clientHeight;
	dispatchRayDesc_.Depth = 1;
}

void DxrCommon::CreateObject() {

	auto vstride = UINT(sizeof(VertexDataDXR));
	auto istride = UINT(sizeof(UINT));

	std::vector<VertexDataDXR> vertices;
	std::vector<UINT>          indices;

	// plane
	Create::Plane(vertices, indices);

	plane_.vertexBuffer = CreateBuffer(
		vstride * vertices.size(),
		vertices.data(),
		D3D12_HEAP_TYPE_DEFAULT,
		D3D12_RESOURCE_FLAG_NONE,
		L"plane_.vertexBuffer"
	);

	plane_.indexBuffer = CreateBuffer(
		istride * indices.size(),
		indices.data(),
		D3D12_HEAP_TYPE_DEFAULT,
		D3D12_RESOURCE_FLAG_NONE,
		L"plane_.indexBuffer"
	);

	plane_.vertexCount = UINT(vertices.size());
	plane_.indexCount = UINT(indices.size());
	plane_.vertexStride = vstride;

	plane_.descriptorVB = CreateStructuredSRV(
		plane_.vertexBuffer.Get(),
		plane_.vertexCount,
		0, vstride
	);

	plane_.descriptorIB = CreateStructuredSRV(
		plane_.indexBuffer.Get(),
		plane_.indexCount,
		0, istride
	);

	plane_.shaderName = kHitGroup_;

	// cube

	vertices.clear();
	indices.clear();

	Create::Cube(vertices, indices);

	cube_.vertexBuffer = CreateBuffer(
		vstride * vertices.size(),
		vertices.data(),
		D3D12_HEAP_TYPE_DEFAULT,
		D3D12_RESOURCE_FLAG_NONE,
		L"cube_.vertexBuffer"
	);

	cube_.indexBuffer = CreateBuffer(
		istride * indices.size(),
		indices.data(),
		D3D12_HEAP_TYPE_DEFAULT,
		D3D12_RESOURCE_FLAG_NONE,
		L"cube_.indexBuffer"
	);

	cube_.vertexCount = UINT(vertices.size());
	cube_.indexCount = UINT(indices.size());
	cube_.vertexStride = vstride;

	cube_.descriptorVB = CreateStructuredSRV(
		cube_.vertexBuffer.Get(),
		cube_.vertexCount,
		0, vstride
	);

	cube_.descriptorIB = CreateStructuredSRV(
		cube_.indexBuffer.Get(),
		cube_.indexCount,
		0, istride
	);

	cube_.shaderName = kHitGroup_;

	vertices.clear();
	indices.clear();

}

void DxrCommon::CreateSceneBLAS() {
	auto planeGeometryDesc = GetGeometryDesc(plane_);
	auto cubeGeometryDesc = GetGeometryDesc(cube_);

	// BLASの作成
	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC asDesc = {};
	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS& inputs = asDesc.Inputs;
	inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
	inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
	inputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_NONE;

	// BLAS を構築するためのバッファを準備 (Plane).
	inputs.NumDescs = 1;
	inputs.pGeometryDescs = &planeGeometryDesc;
	auto planeASB = CreateAccelerationStructure(asDesc);
	planeASB.asbuffer->SetName(L"Plane-Blas");
	asDesc.ScratchAccelerationStructureData = planeASB.scratch->GetGPUVirtualAddress();
	asDesc.DestAccelerationStructureData = planeASB.asbuffer->GetGPUVirtualAddress();

	// コマンドリストに積む.
	auto command = command_->GetCommandList();
	command->BuildRaytracingAccelerationStructure(
		&asDesc, 0, nullptr);

	// BLAS を構築するためのバッファを準備 (Cube).
	inputs.NumDescs = 1;
	inputs.pGeometryDescs = &cubeGeometryDesc;
	auto cubeASB = CreateAccelerationStructure(asDesc);
	cubeASB.asbuffer->SetName(L"Cube-Blas");
	asDesc.ScratchAccelerationStructureData = cubeASB.scratch->GetGPUVirtualAddress();
	asDesc.DestAccelerationStructureData = cubeASB.asbuffer->GetGPUVirtualAddress();

	// コマンドリストに積む.
	command->BuildRaytracingAccelerationStructure(
		&asDesc, 0, nullptr);

	// Plane,Cube それぞれの BLAS についてバリアを設定する.
	D3D12_RESOURCE_BARRIER uavBarriers[] = {
		CD3DX12_RESOURCE_BARRIER::UAV(planeASB.asbuffer.Get()),
		CD3DX12_RESOURCE_BARRIER::UAV(cubeASB.asbuffer.Get()),
	};
	command->ResourceBarrier(_countof(uavBarriers), uavBarriers);
	
	Sent();

	// この先は BLAS のバッファのみ使うのでメンバ変数に代入しておく.
	plane_.blas = planeASB.asbuffer;
	cube_.blas = cubeASB.asbuffer;
}

void DxrCommon::CreateSceneTLAS() {

	std::vector<D3D12_RAYTRACING_INSTANCE_DESC> instanceDescs;
	DeployObjects(instanceDescs);

	// インスタンスの情報を記録したバッファを準備する.
	size_t sizeOfInstanceDescs = instanceDescs.size() * sizeof(D3D12_RAYTRACING_INSTANCE_DESC);
	auto instanceDescBuffer = CreateBuffer(
		sizeOfInstanceDescs,
		instanceDescs.data(),
		D3D12_HEAP_TYPE_UPLOAD,
		D3D12_RESOURCE_FLAG_NONE,
		L"InstanceDescBuffer"
	);

	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC asDesc{};
	auto& inputs = asDesc.Inputs;
	inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;
	inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
	inputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_NONE;

	inputs.NumDescs = UINT(instanceDescs.size());
	inputs.InstanceDescs = instanceDescBuffer->GetGPUVirtualAddress();

	auto sceneASB = CreateAccelerationStructure(asDesc);
	sceneASB.asbuffer->SetName(L"Scene-Tlas");
	asDesc.ScratchAccelerationStructureData = sceneASB.scratch->GetGPUVirtualAddress();
	asDesc.DestAccelerationStructureData = sceneASB.asbuffer->GetGPUVirtualAddress();

	// コマンドリストに積む.
	auto command = command_->GetCommandList();
	command->BuildRaytracingAccelerationStructure(&asDesc, 0, nullptr);

	// TLAS に対しての UAV バリアを設定.
	D3D12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::UAV(
		sceneASB.asbuffer.Get()
	);
	Sent();

	// この先は TLAS のバッファのみ使うのでメンバ変数に代入しておく.
	tlas_ = sceneASB.asbuffer;

	// ディスクリプタの準備.
	tlasDescriptor_ = descriptorHeaps_->GetCurrentDescriptor(SRV);

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_RAYTRACING_ACCELERATION_STRUCTURE;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.RaytracingAccelerationStructure.Location = tlas_->GetGPUVirtualAddress();

	devices_->GetDevice()->CreateShaderResourceView(
		nullptr, &srvDesc, tlasDescriptor_.handleCPU
	);

}

void DxrCommon::DeployObjects(std::vector<D3D12_RAYTRACING_INSTANCE_DESC>& instanceDescs) {

	D3D12_RAYTRACING_INSTANCE_DESC baseDesc = {};
	baseDesc.InstanceMask = 0xFF;
	baseDesc.Flags = D3D12_RAYTRACING_INSTANCE_FLAG_NONE;

	// シーンに床, Cube 2つを設置する.
	instanceDescs.resize(2);
	auto& floor = instanceDescs[0];
	auto& cube = instanceDescs[1];

	
	floor = baseDesc;
	Matrix4x4 mat = Matrix::MakeAffine(unitVector, {0.0f, 1.0f, 0.0f}, {2.0f, 0.0f, 1.0f});
	mat = Matrix::Transpose(mat);
	memcpy(floor.Transform, &mat, sizeof(baseDesc.Transform));
	floor.InstanceContributionToHitGroupIndex = 0;
	floor.InstanceID = 1;
	floor.AccelerationStructure = plane_.blas->GetGPUVirtualAddress();

	cube = baseDesc;
	Matrix4x4 cubeMat = Matrix::MakeAffine({0.5f, 0.5f, 0.5f}, {0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 0.5f});
	cubeMat = Matrix::Transpose(cubeMat);
	memcpy(cube.Transform, &cubeMat, sizeof(baseDesc.Transform));
	cube.InstanceContributionToHitGroupIndex = 1;
	cube.InstanceID = 0;
	cube.AccelerationStructure = cube_.blas->GetGPUVirtualAddress();

}

void DxrCommon::CreateGlobalRootSignature() {
	std::array<CD3DX12_ROOT_PARAMETER, 1> rootParams;

	// TLAS を t0 レジスタに割り当てて使用する設定.
	CD3DX12_DESCRIPTOR_RANGE descRangeTLAS;
	descRangeTLAS.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);

	rootParams[0].InitAsDescriptorTable(1, &descRangeTLAS);

	D3D12_ROOT_SIGNATURE_DESC rootSigDesc{};
	rootSigDesc.NumParameters = UINT(rootParams.size());
	rootSigDesc.pParameters = rootParams.data();

	rootSignatureGlobal_ = CreateRootSignature(devices_->GetDevice(), rootSigDesc);
	rootSignatureGlobal_->SetName(L"RootSignatureGlobal");
}

void DxrCommon::CreateLoaclRootSignatureRayGen() {
	// UAV (u0)
	D3D12_DESCRIPTOR_RANGE descUAV = {};
	descUAV.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
	descUAV.BaseShaderRegister = 0;
	descUAV.NumDescriptors = 1;

	std::array<D3D12_ROOT_PARAMETER, 1> rootParams;
	rootParams[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rootParams[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParams[0].DescriptorTable.NumDescriptorRanges = 1;
	rootParams[0].DescriptorTable.pDescriptorRanges = &descUAV;

	ComPtr<ID3DBlob> blob, errBlob;
	D3D12_ROOT_SIGNATURE_DESC rootSigDesc = {};
	rootSigDesc.NumParameters = UINT(rootParams.size());
	rootSigDesc.pParameters = rootParams.data();
	rootSigDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE;
	rayGenRootSignature_ = CreateRootSignature(devices_->GetDevice(), rootSigDesc);
	rayGenRootSignature_->SetName(L"rayGenRootSignature_");
}

void DxrCommon::CreateLoaclRootSignatureColosestHit() {
	// 頂点・インデックスバッファにアクセスするためのローカルルートシグネチャを作る.
	D3D12_DESCRIPTOR_RANGE rangeIB = {};
	rangeIB.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	rangeIB.BaseShaderRegister = 0;
	rangeIB.NumDescriptors = 1;
	rangeIB.RegisterSpace = 1;

	D3D12_DESCRIPTOR_RANGE rangeVB = {};
	rangeVB.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	rangeVB.BaseShaderRegister = 1;
	rangeVB.NumDescriptors = 1;
	rangeVB.RegisterSpace = 1;


	std::array<D3D12_ROOT_PARAMETER, 2> rootParams;
	rootParams[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rootParams[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParams[0].DescriptorTable.NumDescriptorRanges = 1;
	rootParams[0].DescriptorTable.pDescriptorRanges = &rangeIB;

	rootParams[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rootParams[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParams[1].DescriptorTable.NumDescriptorRanges = 1;
	rootParams[1].DescriptorTable.pDescriptorRanges = &rangeVB;

	D3D12_ROOT_SIGNATURE_DESC rootSigDesc = {};
	rootSigDesc.NumParameters = UINT(rootParams.size());
	rootSigDesc.pParameters = rootParams.data();
	rootSigDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE;

	modelRootSignature_ = CreateRootSignature(devices_->GetDevice(), rootSigDesc);
	modelRootSignature_->SetName(L"model");
}

//=========================================================================================
// create methods
//=========================================================================================

ComPtr<ID3D12Resource> DxrCommon::CreateBuffer(size_t size, const void* data, D3D12_HEAP_TYPE heapType, D3D12_RESOURCE_FLAGS flags, const wchar_t* name) {

	auto device = devices_->GetDevice();

	D3D12_RESOURCE_STATES initState = D3D12_RESOURCE_STATE_COPY_DEST;
	if (heapType == D3D12_HEAP_TYPE_UPLOAD) {
		initState = D3D12_RESOURCE_STATE_GENERIC_READ;
	}

	ComPtr<ID3D12Resource> result = CreateBufferResource(
		device,
		size,
		flags,
		initState,
		heapType,
		name
	);

	if (data != nullptr) {
		if (heapType == D3D12_HEAP_TYPE_DEFAULT) {
			ComPtr<ID3D12Resource> staging;
			
			D3D12_RESOURCE_DESC resDesc{};
			resDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
			resDesc.Width = size;
			resDesc.Height = 1;
			resDesc.DepthOrArraySize = 1;
			resDesc.MipLevels = 1;
			resDesc.SampleDesc = { 1, 0 };
			resDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

			auto hr = device->CreateCommittedResource(
				&kUploadHeapProps,
				D3D12_HEAP_FLAG_NONE,
				&resDesc,
				D3D12_RESOURCE_STATE_GENERIC_READ,
				nullptr,
				IID_PPV_ARGS(staging.ReleaseAndGetAddressOf())
			);
			
			assert(SUCCEEDED(hr));

			WriteToHostVisibleMemory(staging.Get(), data, size);

			auto command = command_->GetCommandList();
			
			command->CopyResource(result.Get(), staging.Get());
			
			Sent();

		} else if (heapType == D3D12_HEAP_TYPE_UPLOAD) {
			WriteToHostVisibleMemory(result.Get(), data, size);
		}
	}

	return result;

}

ComPtr<ID3D12Resource> DxrCommon::CreateBuffer(
	size_t size, D3D12_RESOURCE_FLAGS flags, D3D12_RESOURCE_STATES initialState, D3D12_HEAP_TYPE heapType, const wchar_t* name) {

	D3D12_HEAP_PROPERTIES prop = {};
	if (heapType == D3D12_HEAP_TYPE_DEFAULT) {
		prop = kDefaultHeapProps;
	}
	if (heapType == D3D12_HEAP_TYPE_UPLOAD) {
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
	desc.Flags = flags;

	auto hr = devices_->GetDevice()->CreateCommittedResource(
		&prop,
		D3D12_HEAP_FLAG_NONE,
		&desc,
		initialState,
		nullptr,
		IID_PPV_ARGS(&result)
	);
	
	assert(SUCCEEDED(hr));

	if (result != nullptr && name != nullptr) {
		result->SetName(name);
	}

	return result;

}

DxrObject::Descriptor DxrCommon::CreateStructuredSRV(ID3D12Resource* resource, UINT numElements, UINT firstElement, UINT stride) {
	D3D12_SHADER_RESOURCE_VIEW_DESC desc = {};
	desc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	desc.Format = DXGI_FORMAT_UNKNOWN;
	desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	desc.Buffer.NumElements = numElements;
	desc.Buffer.FirstElement = firstElement;
	desc.Buffer.StructureByteStride = stride;
	
	Descriptor result = descriptorHeaps_->GetCurrentDescriptor(SRV);

	devices_->GetDevice()->CreateShaderResourceView(
		resource,
		&desc,
		result.handleCPU
	);

	return result;

}

D3D12_RAYTRACING_GEOMETRY_DESC DxrCommon::GetGeometryDesc(const PolygonMesh& mesh) {
	D3D12_RAYTRACING_GEOMETRY_DESC desc = {};
	desc.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
	desc.Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE;
	auto& triangles = desc.Triangles;
	triangles.VertexBuffer.StartAddress = mesh.vertexBuffer->GetGPUVirtualAddress();
	triangles.VertexBuffer.StrideInBytes = mesh.vertexStride;
	triangles.VertexCount = mesh.vertexCount;
	triangles.IndexBuffer = mesh.indexBuffer->GetGPUVirtualAddress();
	triangles.IndexCount = mesh.indexCount;
	triangles.VertexFormat = DXGI_FORMAT_R32G32B32_FLOAT;
	triangles.IndexFormat = DXGI_FORMAT_R32_UINT;
	return desc;
}

AccelerationStructureBuffers DxrCommon::CreateAccelerationStructure(
	const D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC& desc) {

	auto device = devices_->GetDevice();
	
	AccelerationStructureBuffers buffer = {};
	// 必要なメモリ数を求める
	D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO info = {};
	device->GetRaytracingAccelerationStructurePrebuildInfo(
		&desc.Inputs, &info
	);

	// スクラッチバッファを確保
	buffer.scratch = CreateBuffer(
		info.ResultDataMaxSizeInBytes,
		D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
		D3D12_HEAP_TYPE_DEFAULT
	);

	buffer.asbuffer = CreateBuffer(
		info.ResultDataMaxSizeInBytes,
		D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
		D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE,
		D3D12_HEAP_TYPE_DEFAULT
	);

	assert(buffer.asbuffer != nullptr || buffer.scratch != nullptr);

	// アップデート用バッファを確保.
	if (desc.Inputs.Flags & D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_UPDATE) {
		buffer.update = CreateBuffer(
			info.UpdateScratchDataSizeInBytes,
			D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
			D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
			D3D12_HEAP_TYPE_DEFAULT
		);
		assert(buffer.update != nullptr);
	}

	return buffer;

}

uint8_t* DxrCommon::WriteShaderRecord(uint8_t* dst, const PolygonMesh& mesh, UINT recordSize) {
	ComPtr<ID3D12StateObjectProperties> rtsoProps;
	stateObject_.As(&rtsoProps);
	auto entryBegin = dst;
	auto shader = mesh.shaderName;
	auto id = rtsoProps->GetShaderIdentifier(shader.c_str());
	if (id == nullptr) {
		assert(false);
	}

	dst += WriteShaderIdentifier(dst, id);
	// 今回のプログラムでは以下の順序でディスクリプタを記録.
	// [0] : インデックスバッファ
	// [1] : 頂点バッファ
	// ※ ローカルルートシグネチャの順序に合わせる必要がある.
	dst += WriteGPUDescriptor(dst, mesh.descriptorIB);
	dst += WriteGPUDescriptor(dst, mesh.descriptorVB);

	dst = entryBegin + recordSize;
	return dst;
}

////////////////////////////////////////////////////////////////////////////////////////////
// Create namespace methods
////////////////////////////////////////////////////////////////////////////////////////////
void Create::Plane(std::vector<VertexDataDXR>& vertex, std::vector<UINT>& index) {
	const Vector4f white = { 1.0f, 1.0f, 1.0f, 1.0f };
	const Vector4f cyan   = { 0.8f, 1.0f, 1.0f, 1.0f };

	VertexDataDXR srcVertexData[] = {
		// position            // normal          // color
		{{-1.0f, -1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f }, cyan },
		{{-1.0f,  1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f }, white },
		{{ 1.0f, -1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f }, cyan },
		{{ 1.0f,  1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f }, white },
	};

	vertex.resize(4);

	for (int i = 0; i < 4; ++i) {
		vertex[i] = srcVertexData[i];
	}

	index = { 0, 1, 2, 2, 1, 3 };

}

void Create::Cube(std::vector<VertexDataDXR>& vertex, std::vector<UINT>& index) {

	vertex.clear();
	index.clear();

	const Vector4f red = { 1.0f, 0.0f, 0.0f, 1.0f };
	const Vector4f green = { 0.0f, 1.0f, 0.0f, 1.0f };
	const Vector4f blue = { 0.0f, 0.0f, 1.0f, 1.0f };
	const Vector4f white = { 1.0f, 1.0f, 1.0f, 1.0f };
	const Vector4f black = { 0.0f, 0.0f, 0.0f, 1.0f };
	const Vector4f yellow = { 1.0f, 1.0f, 0.0f, 1.0f };
	const Vector4f magenta = { 1.0f, 0.0f, 1.0f, 1.0f };
	const Vector4f cyan = { 0.0f, 1.0f, 1.0f, 1.0f };

	vertex = {
		// 裏
		{ {-1.0f,-1.0f,-1.0f}, { 0.0f, 0.0f, -1.0f }, red },
		{ {-1.0f, 1.0f,-1.0f}, { 0.0f, 0.0f, -1.0f }, yellow },
		{ { 1.0f, 1.0f,-1.0f}, { 0.0f, 0.0f, -1.0f }, white },
		{ { 1.0f,-1.0f,-1.0f}, { 0.0f, 0.0f, -1.0f }, magenta },
		// 右
		{ { 1.0f,-1.0f,-1.0f}, { 1.0f, 0.0f, 0.0f }, magenta },
		{ { 1.0f, 1.0f,-1.0f}, { 1.0f, 0.0f, 0.0f }, white},
		{ { 1.0f, 1.0f, 1.0f}, { 1.0f, 0.0f, 0.0f }, cyan},
		{ { 1.0f,-1.0f, 1.0f}, { 1.0f, 0.0f, 0.0f }, blue},
		// 左
		{ {-1.0f,-1.0f, 1.0f}, { -1.0f, 0.0f, 0.0f }, black},
		{ {-1.0f, 1.0f, 1.0f}, { -1.0f, 0.0f, 0.0f }, green},
		{ {-1.0f, 1.0f,-1.0f}, { -1.0f, 0.0f, 0.0f }, yellow},
		{ {-1.0f,-1.0f,-1.0f}, { -1.0f, 0.0f, 0.0f }, red},
		// 正面
		{ { 1.0f,-1.0f, 1.0f}, { 0.0f, 0.0f, 1.0f}, blue},
		{ { 1.0f, 1.0f, 1.0f}, { 0.0f, 0.0f, 1.0f}, cyan},
		{ {-1.0f, 1.0f, 1.0f}, { 0.0f, 0.0f, 1.0f}, green},
		{ {-1.0f,-1.0f, 1.0f}, { 0.0f, 0.0f, 1.0f}, black},
		// 上
		{ {-1.0f, 1.0f,-1.0f}, { 0.0f, 1.0f, 0.0f}, yellow},
		{ {-1.0f, 1.0f, 1.0f}, { 0.0f, 1.0f, 0.0f}, green },
		{ { 1.0f, 1.0f, 1.0f}, { 0.0f, 1.0f, 0.0f}, cyan },
		{ { 1.0f, 1.0f,-1.0f}, { 0.0f, 1.0f, 0.0f}, white},
		// 底
		{ {-1.0f,-1.0f, 1.0f}, { 0.0f, -1.0f, 0.0f}, black},
		{ {-1.0f,-1.0f,-1.0f}, { 0.0f, -1.0f, 0.0f}, red},
		{ { 1.0f,-1.0f,-1.0f}, { 0.0f, -1.0f, 0.0f}, magenta},
		{ { 1.0f,-1.0f, 1.0f}, { 0.0f, -1.0f, 0.0f}, blue},
	};
	index = {
		0, 1, 2, 2, 3,0,
		4, 5, 6, 6, 7,4,
		8, 9, 10, 10, 11, 8,
		12,13,14, 14,15,12,
		16,17,18, 18,19,16,
		20,21,22, 22,23,20,
	};

}
