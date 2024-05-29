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

// Model
#include <Model.h>

////////////////////////////////////////////////////////////////////////////////////////////
// BreakEffect class
////////////////////////////////////////////////////////////////////////////////////////////
class BreakEffect { /* 名前どうにかしたい */
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

private:

	//=========================================================================================
	// private variables
	//=========================================================================================

	std::unique_ptr<Model> screenModel_;

};