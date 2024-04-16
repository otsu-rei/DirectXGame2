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
	class Shader {
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

	};

}