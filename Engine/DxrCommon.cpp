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

	CreateBLAS();
	CreateTLAS();
	CreateRootSignatureGlobal();
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
	commandList->SetComputeRootDescriptorTable(1, outputDescriptor_.handleGPU);

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

	uint32_t payloadSize = static_cast<uint32_t>(sizeof(Vector3f));
	uint32_t attributeSize = static_cast<uint32_t>(sizeof(Vector2f));

	shaderConfig->Config(payloadSize, attributeSize);

	// localがないので飛ばしとく error出たらここが原因かも

	auto grobalRootSignatureSubobject
		= stateObjectDesc.CreateSubobject<CD3DX12_GLOBAL_ROOT_SIGNATURE_SUBOBJECT>();

	grobalRootSignatureSubobject->SetRootSignature(rootSignatureGlobal_.Get());

	auto pipelineConfigSubobject
		= stateObjectDesc.CreateSubobject<CD3DX12_RAYTRACING_PIPELINE_CONFIG_SUBOBJECT>();

	uint32_t maxRecursionDepth = 1; //!< Max31

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
	auto device = devices_->GetDevice();

	// 各シェーダーレコードは Shader Identifier を保持する.
	UINT recordSize = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;

	// グローバルのルートシグネチャ以外の情報を持たないのでレコードサイズはこれだけ.

	// あとはアライメント制約を保つようにする.
	recordSize = Alignment(recordSize, D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT);

	// シェーダーテーブルのサイズを求める.
	UINT raygenSize = 1 * recordSize;   // 今1つの Ray Generation シェーダー.
	UINT missSize = 1 * recordSize;     // 今1つの Miss シェーダー.
	UINT hitGroupSize = 1 * recordSize; // 今1つの HitGroup を使用.

	// 各テーブルの開始位置にアライメント制約があるので調整する.
	auto tableAlign = D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT;
	UINT raygenRegion = Alignment(raygenSize, tableAlign);
	UINT missRegion = Alignment(missSize, tableAlign);
	UINT hitgroupRegion = Alignment(hitGroupSize, tableAlign);

	// シェーダーテーブル確保.
	auto tableSize = raygenRegion + missRegion + hitgroupRegion;
	shaderTable_ = CreateBufferResource(
		device,
		tableSize,
		D3D12_RESOURCE_FLAG_NONE,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		kUploadHeapProps
	);

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
		auto id = rtsoProps->GetShaderIdentifier(kRayGenerationFunctionName_);
		if (id == nullptr) {
			assert(false);
		}
		memcpy(p, id, D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES);
		p += D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;

		// ローカルルートシグネチャ使用時には他のデータを書き込む.
	}

	// Miss Shader 用のシェーダーレコードを書き込み.
	auto missStart = pStart + raygenRegion;
	{
		uint8_t* p = missStart;
		auto id = rtsoProps->GetShaderIdentifier(kMissFunctionName_);
		if (id == nullptr) {
			assert(false);
		}
		memcpy(p, id, D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES);
		p += D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;

		// ローカルルートシグネチャ使用時には他のデータを書き込む.
	}

	// Hit Group 用のシェーダーレコードを書き込み.
	auto hitgroupStart = pStart + raygenRegion + missRegion;
	{
		uint8_t* p = hitgroupStart;
		auto id = rtsoProps->GetShaderIdentifier(kHitGroup_);
		if (id == nullptr) {
			assert(false);
		}
		memcpy(p, id, D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES);
		p += D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;

		// ローカルルートシグネチャ使用時には他のデータを書き込む.
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
	shaderRecordMS.StrideInBytes = recordSize;
	startAddress += missRegion;

	auto& shaderRecordHG = dispatchRayDesc_.HitGroupTable;
	shaderRecordHG.StartAddress = startAddress;
	shaderRecordHG.SizeInBytes = hitGroupSize;
	shaderRecordHG.StrideInBytes = recordSize;
	startAddress += hitgroupRegion;

	dispatchRayDesc_.Width = clientWidth;
	dispatchRayDesc_.Height = clientHeight;
	dispatchRayDesc_.Depth = 1;
}

void DxrCommon::CreateObject() {
	auto device = devices_->GetDevice();

	auto vstrider = sizeof(VertexData);
	auto istrider = sizeof(UINT);

	std::vector<VertexData> vertices;
	std::vector<UINT>       indices;

	Create::Plane(vertices, indices);

	plane_.vertexBuffer = CreateBuffer(
		vstrider * vertices.size(),
		vertices.data(),
		D3D12_HEAP_TYPE_DEFAULT,
		D3D12_RESOURCE_FLAG_NONE,
		L"plane_.vertexBuffer"
	);

	plane_.indexBuffer = CreateBuffer(
		istrider * indices.size(),
		indices.data(),
		D3D12_HEAP_TYPE_DEFAULT,
		D3D12_RESOURCE_FLAG_NONE,
		L"plane_.indexBuffer"
	);

	plane_.vertexCount = UINT(vertices.size());
	plane_.indexCount = UINT(indices.size());
	plane_.vertexStrider = vstrider;

}

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

////////////////////////////////////////////////////////////////////////////////////////////
// Create namespace methods
////////////////////////////////////////////////////////////////////////////////////////////
void Create::Plane(std::vector<VertexData>& vertex, std::vector<UINT>& index) {
	const Vector4f white = { 1.0f, 1.0f, 1.0f, 1.0f };

	VertexData srcVertexData[] = {
		// position            // normal          // color
		{{-1.0f, 0.0f,-1.0f }, { 0.0f, 1.0f, 0.0f }, white },
		{{-1.0f, 0.0f, 1.0f }, { 0.0f, 1.0f, 0.0f }, white },
		{{ 1.0f, 0.0f,-1.0f }, { 0.0f, 1.0f, 0.0f }, white },
		{{ 1.0f, 0.0f, 1.0f }, { 0.0f, 1.0f, 0.0f }, white },
	};

	vertex.resize(4);

	for (int i = 0; i < 4; ++i) {
		vertex[i] = srcVertexData[i];
	}

	index = { 0, 1, 2, 2, 1, 3 };

}
