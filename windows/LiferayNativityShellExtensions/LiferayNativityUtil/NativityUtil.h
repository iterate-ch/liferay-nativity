#pragma once

#include <string>

struct __declspec(dllexport) NativityUtil
{
	using wstring = std::wstring;

	NativityUtil() = delete;

	// Traverses DLL heap boundary, cannot work without ref
	static bool IsFileFiltered(const wstring& file);

	static bool OverlaysEnabled();
	
	// Traverses DLL heap boundary, cannot work without ref
	static bool ReceiveResponse(const wstring& message, wstring& response);
};
