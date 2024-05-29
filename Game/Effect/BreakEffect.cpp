#include "BreakEffect.h"

////////////////////////////////////////////////////////////////////////////////////////////
// BreakEffect class methods
////////////////////////////////////////////////////////////////////////////////////////////

void BreakEffect::Init() {
	screenModel_ = std::make_unique<Model>("./resources/model", "glassScreen.obj");


}

void BreakEffect::Term() {
}

void BreakEffect::Update() {
}

void BreakEffect::Draw(Texture* output, const D3D12_GPU_DESCRIPTOR_HANDLE& handle) {
	output; handle;

}

