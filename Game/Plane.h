#pragma once

//-----------------------------------------------------------------------------------------
// include
//-----------------------------------------------------------------------------------------
#include <memory>
#include "DxBufferResource.h"
#include "DxStructuredBuffer.h"
#include "ObjectStructure.h"
#include "Attribute.h"

////////////////////////////////////////////////////////////////////////////////////////////
// Plane class
////////////////////////////////////////////////////////////////////////////////////////////
class Plane
	: public Attribute {
public:

	//=========================================================================================
	// public methods
	//=========================================================================================

	Plane() { Init(); }

	~Plane() { Term(); }

	void Init();

	void Term();

	void Update();

	void Draw();

	void SetAttributeImGui() override;


private:

	////////////////////////////////////////////////////////////////////////////////////////////
	// PlaneMaterial structure
	////////////////////////////////////////////////////////////////////////////////////////////
	struct PlaneMaterial { //!< FloorMaterial参照
		Vector4f color;
	};

	//=========================================================================================
	// private variables
	//=========================================================================================

	// vertexResource
	std::unique_ptr<DxObject::BufferResource<VertexData>> vertexResource_;
	std::unique_ptr<DxObject::StructuredBuffer> vertexStructuredBuffer_;

	// indexResource
	std::unique_ptr<DxObject::IndexBufferResource> indexResource_;
	std::unique_ptr<DxObject::StructuredBuffer> indexStructuredBuffer_;

	// materialResource
	std::unique_ptr<DxObject::BufferPtrResource<PlaneMaterial>> materialResource_;
	PlaneMaterial material_;

	// matrixResource
	std::unique_ptr<DxObject::BufferResource<TransformationMatrix>> matrixResource_;
	Transform transform_;

	// textures
	std::vector<std::string> textureNames_;
	std::string texture_;

};