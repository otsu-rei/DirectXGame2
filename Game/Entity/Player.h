#pragma once

//-----------------------------------------------------------------------------------------
// include
//-----------------------------------------------------------------------------------------
// DxSrc
#include <DxBufferResource.h>

// Mesh
#include <DxMesh.h>

// Model
#include <Model.h>

// c++
#include <memory>

////////////////////////////////////////////////////////////////////////////////////////////
// Player class
////////////////////////////////////////////////////////////////////////////////////////////
class Player
	: public Attribute {
public:

	//=========================================================================================
	// public methods
	//=========================================================================================

	Player() { Init(); }

	~Player() { Term(); }

	void Init();

	void Term();

	void Update();

	void Draw();

	void SetAttributeImGui() override;

private:

	////////////////////////////////////////////////////////////////////////////////////////////
	// Material structure
	////////////////////////////////////////////////////////////////////////////////////////////
	struct Material {
		Vector4f color = { 1.0f, 1.0f, 1.0f, 1.0f };
	};

	//=========================================================================================
	// private variables
	//=========================================================================================

	// IA
	std::unique_ptr<Model>          model_;
	std::unique_ptr<DxObject::Mesh> mesh_;

	// constantBuffer
	std::unique_ptr<DxObject::BufferResource<TransformationMatrix>> matrixBuffer_;
	Transform transform_;

	std::unique_ptr<DxObject::BufferPtrResource<Material>>          materialBuffer_;
	Material material_;

	//=========================================================================================
	// private methods
	//=========================================================================================

	void Move();

	void UpdateMatrix();

};