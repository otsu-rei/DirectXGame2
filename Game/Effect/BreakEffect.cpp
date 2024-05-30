#include "BreakEffect.h"

//-----------------------------------------------------------------------------------------
// include
//-----------------------------------------------------------------------------------------
#include <Environment.h>
#include <algorithm>
#include <Easing.h>

//-----------------------------------------------------------------------------------------
// using
//-----------------------------------------------------------------------------------------
using namespace DxObject;

////////////////////////////////////////////////////////////////////////////////////////////
// BreakEffect class methods
////////////////////////////////////////////////////////////////////////////////////////////

void BreakEffect::Init() {
	screenModel_ = std::make_unique<Model>("./resources/model", "glassScreen.obj");

	matrixs_.resize(screenModel_->GetSize());
	transforms_.resize(screenModel_->GetSize());

	for (uint32_t i = 0; i < matrixs_.size(); ++i) {
		matrixs_[i] = std::make_unique<BufferResource<TransformationMatrix>>(MyEngine::GetDevicesObj(), 1);
		matrixs_[i]->operator[](0).world                 = Matrix::MakeAffine(scale_, origin, origin);
		matrixs_[i]->operator[](0).worldInverseTranspose = Matrix4x4::MakeIdentity();
	}

	cameraBuffer_ = std::make_unique<BufferResource<Matrix4x4>>(MyEngine::GetDevicesObj(), 1);
	cameraBuffer_->operator[](0) = Matrix::MakeOrthographic(0.0f, 0.0f, static_cast<float>(kWindowWidth), static_cast<float>(kWindowHeight), 0.0f, 100.0f);

	SetThisAttribute("Effect");

}

void BreakEffect::Term() {
}

void BreakEffect::Update() {

	if (isStart_) {
		t_ += 0.01f;
		t_ = std::clamp(t_, 0.0f, 1.0f);

		if (t_ == 1.0f) {
			isStart_ = false;
		}
	}

	for (uint32_t i = 0; i < screenModel_->GetSize(); ++i) {

		float t_2 = LerpDivision(EaseInQuint(t_), screenModel_->GetSize(), easeingPoint_, i);
		float rotateT = LerpDivision(EaseInOutQuad(t_), screenModel_->GetSize(), easeingPoint_, i);

		transforms_[i].translate.y = std::lerp(0.0f, -0.02f, t_2);
		transforms_[i].rotate.x = std::lerp(0.0f, -std::numbers::pi_v<float> / 2.0f, rotateT);
		matrixs_[i]->operator[](0).world = Matrix::MakeAffine(scale_, transforms_[i].rotate, transforms_[i].translate);
	}
}

void BreakEffect::Draw(Texture* output, const D3D12_GPU_DESCRIPTOR_HANDLE& handle) {

	auto commandList = MyEngine::GetCommandList();

	MyEngine::BeginOffScreen(output);

	MyEngine::SetBlendMode(kBlendModeNormal);
	MyEngine::SetPipelineType(BREAKEFFECT);
	MyEngine::SetPipelineState();

	for (uint32_t i = 0; i < screenModel_->GetSize(); ++i) {

		screenModel_->SetBuffers(commandList, i);

		commandList->SetGraphicsRootConstantBufferView(0, matrixs_[i]->GetGPUVirtualAddress());
		commandList->SetGraphicsRootConstantBufferView(1, cameraBuffer_->GetGPUVirtualAddress());
		commandList->SetGraphicsRootDescriptorTable(2, handle);

		screenModel_->DrawCall(commandList, i, 1);
	}

	MyEngine::EndOffScreen();
}

void BreakEffect::SetAttributeImGui() {

	ImGui::DragFloat("easingPoint", &easeingPoint_, 0.1f, 0.0f, 100.0f);
	ImGui::SliderFloat("t", &t_, 0.0f, 1.0f);
	if (ImGui::Button("reset")) {
		t_ = 0.0f;
	}

	ImGui::SameLine();

	if (ImGui::Button("start")) {
		isStart_ = true;
	}

}

float LerpDivision(float t, uint32_t division, float easePoint, int currentIndex) {
	
	float t_2 = t * ((division + easePoint) / easePoint) - static_cast<float>((currentIndex) / easePoint);
	return std::clamp(t_2, 0.0f, 1.0f);
}
