#pragma once

#include <string>

struct __declspec(dllexport) NativityUtil
{
	using wstring = std::wstring;

	NativityUtil() = delete;

	static bool IsFileFiltered(const wstring file);

	static bool OverlaysEnabled();
	
	static bool ReceiveResponse(const wstring message, wstring& const response);
};
