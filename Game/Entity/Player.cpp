#include "Player.h"

//-----------------------------------------------------------------------------------------
// using
//-----------------------------------------------------------------------------------------
using namespace DxObject;

////////////////////////////////////////////////////////////////////////////////////////////
// Player class methods
////////////////////////////////////////////////////////////////////////////////////////////

void Player::Init() {

	// IA
	model_ = std::make_unique<Model>("./resources/model", "Player.obj");
	mesh_ = std::make_unique<Mesh>(model_->GetMeshData(0).vertexResource.get(), model_->GetMeshData(0).indexResource.get());

	// matrix
	matrixBuffer_ = std::make_unique<BufferResource<TransformationMatrix>>(MyEngine::GetDevicesObj(), 1);
	matrixBuffer_->operator[](0).world                 = Matrix4x4::MakeIdentity();
	matrixBuffer_->operator[](0).worldInverseTranspose = Matrix4x4::MakeIdentity();

	transform_.scale = { 0.2f, 0.2f, 0.2f };

	// material
	materialBuffer_ = std::make_unique<BufferPtrResource<Material>>(MyEngine::GetDevicesObj(), 1);
	materialBuffer_->SetPtr(0, &material_);

	SetThisAttribute("player");

}

void Player::Term() {
}

void Player::Update() {

	Move();
	UpdateMatrix();

}

void Player::Draw() {

	MyEngine::SetBlendMode(kBlendModeNormal);
	MyEngine::SetPipelineType(PLAYER);
	MyEngine::SetPipelineState();

	auto commandList = MyEngine::GetCommandList();

	commandList->SetGraphicsRootConstantBufferView(4, matrixBuffer_->GetGPUVirtualAddress());
	commandList->SetGraphicsRootConstantBufferView(5, materialBuffer_->GetGPUVirtualAddress());
	commandList->SetGraphicsRootConstantBufferView(6, MyEngine::camera3D->GetGPUVirtualAddress());

	mesh_->Dispatch(0, 1, 2, 3);

}

void Player::SetAttributeImGui() {
	transform_.SetImGuiCommand(0.02f);

	ImGui::ColorEdit4("color", &material_.color.x);
}

void Player::Move() {

	Vector3f velocity = { 0.0f };

	if (MyEngine::IsPressKey(DIK_W)) {
		velocity.z += 0.02f;
	}

	if (MyEngine::IsPressKey(DIK_S)) {
		velocity.z -= 0.02f;
	}

	if (MyEngine::IsPressKey(DIK_A)) {
		velocity.x -= 0.02f;
	}

	if (MyEngine::IsPressKey(DIK_D)) {
		velocity.x += 0.02f;
	}

	static float frame = 0.0f;
	frame += 0.04f;
	transform_.translate.y = -std::cos(frame) * 0.02f;

	transform_.translate += velocity;

}

void Player::UpdateMatrix() {

	matrixBuffer_->operator[](0).world = Matrix::MakeAffine(transform_.scale, transform_.rotate, transform_.translate);
	// todo: worldInverseTransposeの設定
}
