#pragma once

//-----------------------------------------------------------------------------------------
// include
//-----------------------------------------------------------------------------------------
// DxObject
#include <DxBufferResource.h>

// c++
#include <cstdint>
#include <memory>

#include <ObjectStructure.h>
#include <DrawMethod.h>

////////////////////////////////////////////////////////////////////////////////////////////
// Sphere class
////////////////////////////////////////////////////////////////////////////////////////////
class Sphere {
public:

	Sphere() { Init(); }

	~Sphere() { Term(); }

	void Init();

	void Term();

	void Update();

	void Draw();

	void SetOnImGui();

private:

	//=========================================================================================
	// private variables
	//=========================================================================================

	DrawData drawData_;

	// matrixResource
	std::unique_ptr<DxObject::BufferResource<TransformationMatrix>> matrixResource_;
	Transform transform_;


};