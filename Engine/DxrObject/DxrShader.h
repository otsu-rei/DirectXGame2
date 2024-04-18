#pragma once

//-----------------------------------------------------------------------------------------
// include
//-----------------------------------------------------------------------------------------
// directX
#include <d3d12.h>
#include <dxgi1_6.h>
#include <dxcapi.h>

// c++
#include <cassert>

#include <ComPtr.h>

//-----------------------------------------------------------------------------------------
// comment
//-----------------------------------------------------------------------------------------
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxcompiler.lib")

////////////////////////////////////////////////////////////////////////////////////////////
// DxrObject namespace
////////////////////////////////////////////////////////////////////////////////////////////
namespace DxrObject {

	////////////////////////////////////////////////////////////////////////////////////////////
	// Shader class
	////////////////////////////////////////////////////////////////////////////////////////////
	class Shader { // stateObject
	public:

		Shader() { Init(); }

		~Shader() { Term(); }

		void Init();

		void Term();

	private:

		ComPtr<IDxcLibrary> library_;
		ComPtr<IDxcCompiler> compiler_;

		ComPtr<IDxcBlob> shaderBlob_;

		static const LPCWSTR kShaderFileName_;
		static const LPCWSTR kShaderModel_;

		static const LPCWSTR kDefaultHitGroup_;

		// stateObject
		D3D12_STATE_SUBOBJECT subObjects[32];

	};

}