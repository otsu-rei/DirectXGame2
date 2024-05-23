#pragma once

//-----------------------------------------------------------------------------------------
// include
//-----------------------------------------------------------------------------------------
// c++
#include <cmath>
#include <numbers>
#include <memory>

// DxObject
#include <DxBufferResource.h>

// structure
#include <ObjectStructure.h>

////////////////////////////////////////////////////////////////////////////////////////////
// DrawData structure
////////////////////////////////////////////////////////////////////////////////////////////
struct DrawData {
	std::unique_ptr<DxObject::BufferResource<VertexData>> vertex;
	std::unique_ptr<DxObject::IndexBufferResource>        index;

	void Reset() {
		vertex.reset();
		index.reset();
	}
};

////////////////////////////////////////////////////////////////////////////////////////////
// DrawMethod namespace
////////////////////////////////////////////////////////////////////////////////////////////
namespace DrawMethods {

	DrawData Sphere(float radius, uint32_t kSubdivision);
	DrawData Plane(const Vector2f& size);
};