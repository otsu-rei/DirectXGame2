#include "Logger.h"

//-----------------------------------------------------------------------------------------
// include
//-----------------------------------------------------------------------------------------
#include <windows.h>
#include <assert.h>

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
	OutputDebugStringA(log.c_str());
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
