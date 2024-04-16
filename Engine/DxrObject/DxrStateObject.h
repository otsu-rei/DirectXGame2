#pragma once

//-----------------------------------------------------------------------------------------
// include
//-----------------------------------------------------------------------------------------
// directX
#include <d3d12.h>
#include <dxgi1_6.h>

// c++
#include <cstdint>

// ComPtr
#include <ComPtr.h>

//-----------------------------------------------------------------------------------------
// comment
//-----------------------------------------------------------------------------------------
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

////////////////////////////////////////////////////////////////////////////////////////////
// DxrObject namespace
////////////////////////////////////////////////////////////////////////////////////////////
namespace DxrObject {

	////////////////////////////////////////////////////////////////////////////////////////////
	// StateObject class
	////////////////////////////////////////////////////////////////////////////////////////////
	class StateObject {
	public:

		StateObject() { Init(); }

		~StateObject() { Term(); }

		void Init();

		void Term();

	private:

		static const LPCWSTR kRayGenerationShaderName_;
		static const LPCWSTR kClosesetHitShaderName_;
		static const LPCWSTR kMissShader_;
		static const LPCWSTR kHitGroup_;
		

	};

}