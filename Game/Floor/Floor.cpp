#include "Floor.h"

//-----------------------------------------------------------------------------------------
// include
//-----------------------------------------------------------------------------------------
#include "MyEngine.h"
#include "DirectXCommon.h"
#include "Console.h"

// test include

//-----------------------------------------------------------------------------------------
// using
//-----------------------------------------------------------------------------------------
using namespace DxObject;

////////////////////////////////////////////////////////////////////////////////////////////
// Floor class methods
////////////////////////////////////////////////////////////////////////////////////////////

void Floor::Init() {

	// attribute base object
	name_ = "Floor";
	Console::GetInstance()->SetAttribute(this);

	// vertices
	vertices_[LEFTBOTTOM].position  = { -floorSize_.x / 2.0f, 0.0f, -floorSize_.y / 2.0f };
	vertices_[LEFTTOP].position     = { -floorSize_.x / 2.0f, 0.0f, floorSize_.y / 2.0f };
	vertices_[RIGHTTOP].position    = { floorSize_.x / 2.0f, 0.0f, floorSize_.y / 2.0f };
	vertices_[RIGHTBOTTOM].position = { floorSize_.x / 2.0f, 0.0f, -floorSize_.y / 2.0f };

	for (uint32_t i = 0; i < kCountOfVertexType; ++i) {
		vertices_[i].normal   = { 0.0f, 1.0f, 0.0f };
		vertices_[i].texcoord = { vertices_[i].position.x * tileScale, vertices_[i].position.z * tileScale };
	}

	// vertexResource
	vertexResource_ = std::make_unique<DxObject::BufferPtrResource<VertexData>>(MyEngine::GetDevicesObj(), kCountOfVertexType);

	for (uint32_t i = 0; i < kCountOfVertexType; ++i) {
		vertexResource_->SetPtr(i, &vertices_[i]);
	}

	vertexStructuredBuffer_ = std::make_unique<DxObject::StructuredBuffer>(vertexResource_.get());

	// indexResource
	indexResource_ = std::make_unique<DxObject::IndexBufferResource>(MyEngine::GetDevicesObj(), 6);
	indexResource_->operator[](0) = 0;
	indexResource_->operator[](1) = 1;
	indexResource_->operator[](2) = 2;

	indexResource_->operator[](3) = 1;
	indexResource_->operator[](4) = 3;
	indexResource_->operator[](5) = 2;

	indexStructuredBuffer_ = std::make_unique<DxObject::StructuredBuffer>(indexResource_.get());

	// material
	material_.color = { 1.0f, 1.0f, 1.0f, 1.0f };

	// materialResource
	materialResource_ = std::make_unique<DxObject::BufferPtrResource<FloorMaterial>>(MyEngine::GetDevicesObj(), 1);
	materialResource_->SetPtr(0, &material_);

	// matrixResources
	matrixResource_ = std::make_unique<DxObject::BufferResource<TransformationMatrix>>(MyEngine::GetDevicesObj(), 1);
	matrixResource_->operator[](0).world = Matrix4x4::MakeIdentity();
	matrixResource_->operator[](0).wvp = Matrix4x4::MakeIdentity();
	matrixResource_->operator[](0).worldInverseTranspose = Matrix4x4::MakeIdentity();

	Console::GetInstance()->SetLog("Floor complate cleate");
}

void Floor::Update() {

	// matrix
	Matrix4x4 world = Matrix::MakeAffine(transform_.scale, transform_.rotate, transform_.translate);


	matrixResource_->operator[](0).world = world;
	matrixResource_->operator[](0).wvp = world * MyEngine::camera3D_->GetViewProjectionMatrix();
	matrixResource_->operator[](0).worldInverseTranspose = world;

	// vertices
	vertices_[LEFTBOTTOM].position = { -floorSize_.x / 2.0f, 0.0f, -floorSize_.y / 2.0f };
	vertices_[LEFTTOP].position = { -floorSize_.x / 2.0f, 0.0f, floorSize_.y / 2.0f };
	vertices_[RIGHTTOP].position = { floorSize_.x / 2.0f, 0.0f, floorSize_.y / 2.0f };
	vertices_[RIGHTBOTTOM].position = { floorSize_.x / 2.0f, 0.0f, -floorSize_.y / 2.0f };

	for (uint32_t i = 0; i < kCountOfVertexType; ++i) {
		vertices_[i].texcoord = { vertices_[i].position.x * tileScale, -vertices_[i].position.z * tileScale };
	}
}

void Floor::Draw() {

	MyEngine::SetBlendMode();
	MyEngine::SetPipelineType(FLOOR);
	MyEngine::SetPipelineState();

	// commandListの取り出し
	auto commandList = MyEngine::GetCommandList();

	/*D3D12_VERTEX_BUFFER_VIEW vertexBufferView = vertexResource_->GetVertexBufferView();
	D3D12_INDEX_BUFFER_VIEW indexBufferView = indexResource_->GetIndexBufferView();

	commandList->IASetVertexBuffers(0, 1, &vertexBufferView);
	commandList->IASetIndexBuffer(&indexBufferView);*/
	ID3D12DescriptorHeap* srv[] = { MyEngine::GetDxCommon()->GetDescriptorsObj()->GetDescriptorHeap(SRV) };
	commandList->SetDescriptorHeaps(1, srv);

	// ParamBuffers
	commandList->SetGraphicsRootConstantBufferView(0, matrixResource_->GetGPUVirtualAddress());
	commandList->SetGraphicsRootConstantBufferView(1, materialResource_->GetGPUVirtualAddress());
	commandList->SetGraphicsRootDescriptorTable(2, MyEngine::GetTextureHandleGPU("resources/tile_black.png"));

	
	// mesh param
	commandList->SetGraphicsRootShaderResourceView(3, vertexResource_->GetGPUVirtualAddress());
	commandList->SetGraphicsRootShaderResourceView(4, indexResource_->GetGPUVirtualAddress());

	/*commandList->DrawIndexedInstanced(indexResource_->GetSize(), 1, 0, 0, 0);*/
	commandList->DispatchMesh(2, 1, 1);

}

void Floor::Term() {
	vertexResource_.reset();
	vertexStructuredBuffer_.reset();
	indexResource_.reset();
	indexStructuredBuffer_.reset();

	materialResource_.reset();
	matrixResource_.reset();
}

void Floor::SetOnImGui() {
	if (ImGui::TreeNode("Floor")) {
		ImGui::DragFloat2("size", &floorSize_.x, 0.01f);
		ImGui::DragFloat("tileScale", &tileScale, 0.01f, 0.0f, 1000.0f);

		ImGui::DragFloat3("translate", &transform_.translate.x, 0.1f);

		ImGui::TreePop();
	}

	Update();
}

void Floor::SetAttributeImGui() {
	ImGui::DragFloat2("size", &floorSize_.x, 0.01f);
	ImGui::DragFloat("tileScale", &tileScale, 0.01f, 0.0f, 1000.0f);

	ImGui::DragFloat3("translate", &transform_.translate.x, 0.1f);


	Update();
}