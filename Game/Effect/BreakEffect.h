#pragma once

//-----------------------------------------------------------------------------------------
// include
//-----------------------------------------------------------------------------------------
// texture
#include <TextureManager.h>

// DxObject
#include <DxBufferResource.h>

#include <ObjectStructure.h>

// c++
#include <cstdint>
#include <memory>
#include <vector>

// Model
#include <Model.h>

#include <Environment.h>

////////////////////////////////////////////////////////////////////////////////////////////
// BreakEffect class
////////////////////////////////////////////////////////////////////////////////////////////
class BreakEffect
	: public Attribute { /* 名前どうにかしたい */
public:

	//=========================================================================================
	// public methods
	//=========================================================================================

	BreakEffect() { Init(); }

	~BreakEffect() { Term(); }

	void Init();

	void Term();

	void Update();

	void Draw(Texture* output, const D3D12_GPU_DESCRIPTOR_HANDLE& handle);
	
	void SetAttributeImGui() override;

private:

	//=========================================================================================
	// private variables
	//=========================================================================================

	// IA
	std::unique_ptr<Model> screenModel_;

	std::vector<std::unique_ptr<DxObject::BufferResource<TransformationMatrix>>> matrixs_;
	std::vector<Transform> transforms_;
	
	const Vector3f scale_ = { kWindowWidth / (16.0f * 2.0f), kWindowHeight / (9.0f * 2.0f), 1.0f};

	// cbv camera2D
	std::unique_ptr<DxObject::BufferResource<Matrix4x4>> cameraBuffer_;

	// effect 
	float t_ = 0.0f;
	float easeingPoint_ = 10.0f;

	bool isStart_ = false;

};

float LerpDivision(float t, uint32_t division, float easePoint, int currentIndex);