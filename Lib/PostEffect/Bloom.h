#pragma once

//-----------------------------------------------------------------------------------------
// include
//-----------------------------------------------------------------------------------------
// Texture
#include <TextureManager.h>

// 

////////////////////////////////////////////////////////////////////////////////////////////
// Bloom class
////////////////////////////////////////////////////////////////////////////////////////////
class Bloom {
public:

	//=========================================================================================
	// public methods
	//=========================================================================================

	Bloom() { Init(); }

	~Bloom() { Term(); }

	void Init();

	void Term();

	

private:
	
	//=========================================================================================
	// private variables
	//=========================================================================================

	/* textures */
	std::unique_ptr<RenderTexture> mainRenderingTexture_; 
	std::unique_ptr<RenderTexture> luminanceTexture_;
	std::unique_ptr<RenderTexture> blurTexture_;

};