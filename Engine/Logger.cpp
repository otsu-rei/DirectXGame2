#include "Logger.h"

//-----------------------------------------------------------------------------------------
// include
//-----------------------------------------------------------------------------------------
#include <comdef.h>
#include <assert.h>
#include <sstream>
#include <iomanip>

//-----------------------------------------------------------------------------------------
// methods
//-----------------------------------------------------------------------------------------

std::wstring ToWstring(const std::string& str) {
	if (str.empty()) {
		return std::wstring();
	}

	auto sizeNeeded = MultiByteToWideChar(CP_UTF8, 0, reinterpret_cast<const char*>(&str[0]), static_cast<int>(str.size()), NULL, 0);
	if (sizeNeeded == 0) {
		return std::wstring();
	}
	std::wstring result(sizeNeeded, 0);
	MultiByteToWideChar(CP_UTF8, 0, reinterpret_cast<const char*>(&str[0]), static_cast<int>(str.size()), &result[0], sizeNeeded);
	return result;

}

std::string ToString(const std::wstring& str) {
	if (str.empty()) {
		return std::string();
	}

	auto sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, str.data(), static_cast<int>(str.size()), NULL, 0, NULL, NULL);
	if (sizeNeeded == 0) {
		return std::string();
	}
	std::string result(sizeNeeded, 0);
	WideCharToMultiByte(CP_UTF8, 0, str.data(), static_cast<int>(str.size()), result.data(), sizeNeeded, NULL, NULL);
	return result;

}

void Log(const std::string& log) {
	std::string output = log + "\n";
	OutputDebugStringA(output.c_str());
	ExternalLogger::Write(log);
}

void Log(const std::wstring& logW) {
	std::string str = ToString(logW);
	Log(str);
}

void Assert(bool isSuccess, const std::string& errorLog) {
	if (!isSuccess) {
		MessageBoxW(nullptr, ToWstring(errorLog).c_str(), L"Error Message", 0);
		assert(isSuccess);
		exit(1);
	}
}

void Assert(bool isSuccess, const std::string& errorLog, const std::string& textTitle) {
	if (!isSuccess) {
		MessageBoxW(nullptr, ToWstring(errorLog).c_str(), ToWstring(textTitle).c_str(), 0);
		assert(isSuccess);
		exit(1);
	}
}

void AssertHRESULT(const HRESULT& hr) {
	if (FAILED(hr)) {
		// error mes の出力 
		_com_error err(hr);
		LPCTSTR errMsg = err.ErrorMessage();

		std::wstring wErrMsg(errMsg);

		ExternalLogger::Write("Error: " + ToString(wErrMsg));

		// error codeの出力
		std::ostringstream oss;
		oss << "Error code: 0x" << std::hex << hr;

		ExternalLogger::Write(oss.str());

		assert(false);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////
// ExternalLogger class
////////////////////////////////////////////////////////////////////////////////////////////

bool ExternalLogger::isOutput_ = false;

const std::string ExternalLogger::filename_ = "EngineLog";
std::ofstream ExternalLogger::file_;

void ExternalLogger::Open() {
	if (!isOutput_) {
		return;
	}

	std::string label = filename_ + ".md"; // markdown形式で書き込み 
	file_.open(label);
}

void ExternalLogger::Close() {
	file_.close();
}

void ExternalLogger::Write(const std::string& log) {
	if (!isOutput_) {
		return;
	}

	file_ << log + "\n";
	file_.flush();
}
