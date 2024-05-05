#include "Sphere.h"

//-----------------------------------------------------------------------------------------
// include
//-----------------------------------------------------------------------------------------
#include <MyEngine.h>

////////////////////////////////////////////////////////////////////////////////////////////
// Sphere class methods
////////////////////////////////////////////////////////////////////////////////////////////

void Sphere::Init() {

	drawData_ = DrawMethods::Sphere(1.0f, 16);

	// matrixResources
	matrixResource_ = std::make_unique<DxObject::BufferResource<TransformationMatrix>>(MyEngine::GetDevicesObj(), 1);
	matrixResource_->operator[](0).world = Matrix4x4::MakeIdentity();
	matrixResource_->operator[](0).wvp = Matrix4x4::MakeIdentity();
	matrixResource_->operator[](0).worldInverseTranspose = Matrix4x4::MakeIdentity();

}

void Sphere::Term() {
	drawData_.Reset();

	matrixResource_.reset();
}

void Sphere::Update() {
	// matrix
	Matrix4x4 world = Matrix::MakeAffine(transform_.scale, transform_.rotate, transform_.translate);

	matrixResource_->operator[](0).world = world;
	matrixResource_->operator[](0).wvp = world * MyEngine::camera3D_->GetViewProjectionMatrix();
	matrixResource_->operator[](0).worldInverseTranspose = world;
}

void Sphere::Draw() {
	MyEngine::SetBlendMode();
	MyEngine::SetPipelineType(SPHERE);
	MyEngine::SetPipelineState();

	// commandListの取り出し
	ID3D12GraphicsCommandList* commandList = MyEngine::GetCommandList();

	D3D12_VERTEX_BUFFER_VIEW vertexBufferView = drawData_.vertex->GetVertexBufferView();
	D3D12_INDEX_BUFFER_VIEW indexBufferView = drawData_.index->GetIndexBufferView();

	commandList->IASetVertexBuffers(0, 1, &vertexBufferView);
	commandList->IASetIndexBuffer(&indexBufferView);

	// ParamBuffers
	commandList->SetGraphicsRootConstantBufferView(0, matrixResource_->GetGPUVirtualAddress());

	commandList->DrawIndexedInstanced(drawData_.index->GetSize(), 1, 0, 0, 0);
}

void Sphere::SetOnImGui() {
	if (ImGui::TreeNode("Sphere")) {

		transform_.SetImGuiCommand();

		ImGui::TreePop();
	}

	Update();
}
