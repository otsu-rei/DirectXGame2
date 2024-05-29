#include "Sphere.h"

//-----------------------------------------------------------------------------------------
// include
//-----------------------------------------------------------------------------------------
#include <Console.h>
#include <MyEngine.h>
#include <DrawMethod.h>
#include <Model.h>

//-----------------------------------------------------------------------------------------
// using
//-----------------------------------------------------------------------------------------
using namespace DxObject;

////////////////////////////////////////////////////////////////////////////////////////////
// Sphere class methods
////////////////////////////////////////////////////////////////////////////////////////////

void Sphere::Init() {

	SetThisAttribute("sphere");

	// mesh
	{
		// sphereのvertex, indexの取得
		std::unique_ptr<Model> model = std::make_unique<Model>("resources/model", "sphere.obj");

		// meshに書き込み
		mesh_ = std::make_unique<Mesh>(model->GetMeshData(0).vertexResource.get(), model->GetMeshData(0).indexResource.get());

		// 不要になったのでreset
		model.reset();
	}

	// material
	material_.color = { 1.0f, 1.0f, 1.0f, 1.0f };

	// materialResource
	materialResource_ = std::make_unique<DxObject::BufferPtrResource<SphereMaterial>>(MyEngine::GetDevicesObj(), 1);
	materialResource_->SetPtr(0, &material_);

	// matrixResources
	matrixResource_ = std::make_unique<DxObject::BufferResource<TransformationMatrix>>(MyEngine::GetDevicesObj(), 1);
	matrixResource_->operator[](0).world = Matrix4x4::MakeIdentity();
	matrixResource_->operator[](0).worldInverseTranspose = Matrix4x4::MakeIdentity();

}

void Sphere::Term() {
	mesh_.reset();
	materialResource_.reset();
	matrixResource_.reset();
}

void Sphere::Update() {

	Vector3f velocity = { 0.0f };

	if (MyEngine::IsPressKey(DIK_W)) {
		velocity.y += 0.2f;
	}

	if (MyEngine::IsPressKey(DIK_S)) {
		velocity.y -= 0.2f;
	}

	if (MyEngine::IsPressKey(DIK_A)) {
		velocity.x -= 0.2f;
	}

	if (MyEngine::IsPressKey(DIK_D)) {
		velocity.x += 0.2f;
	}

	transform_.translate += velocity;

	// matrix
	Matrix4x4 world = Matrix::MakeAffine(transform_.scale, transform_.rotate, transform_.translate);

	matrixResource_->operator[](0).world = world;
	matrixResource_->operator[](0).worldInverseTranspose = world;
}

void Sphere::Draw() {

	MyEngine::SetBlendMode(kBlendModeNormal);
	MyEngine::SetPipelineType(DEFAULT);
	MyEngine::SetPipelineState();

	auto commandList = MyEngine::GetCommandList();

	// ParamBuffers
	commandList->SetGraphicsRootConstantBufferView(4, matrixResource_->GetGPUVirtualAddress());
	commandList->SetGraphicsRootConstantBufferView(5, materialResource_->GetGPUVirtualAddress());
	commandList->SetGraphicsRootConstantBufferView(6, MyEngine::camera3D->GetGPUVirtualAddress());
	commandList->SetGraphicsRootDescriptorTable(7, MyEngine::GetTextureHandleGPU("resources/uvChecker.png"));

	mesh_->Dispatch(0, 1, 2, 3);

}

void Sphere::SetAttributeImGui() {
	ImGui::DragFloat3("scale",     &transform_.scale.x,     0.01f);
	ImGui::DragFloat3("rotate",    &transform_.rotate.x,    0.01f);
	ImGui::DragFloat3("translate", &transform_.translate.x, 0.01f);
	ImGui::ColorEdit4("color",     &material_.color.r);
}
