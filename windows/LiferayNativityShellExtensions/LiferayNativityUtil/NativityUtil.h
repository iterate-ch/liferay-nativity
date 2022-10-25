#pragma once

#include <comdef.h>
#include <Windows.h>
#include <string>

inline void throw_if_fail(HRESULT hr)
{
	if (FAILED(hr))
	{
		throw _com_error(hr);
	}
}

struct __declspec(dllexport) NativityUtil
{
	NativityUtil() = delete;

	// Traverses DLL heap boundary, cannot work without ref
	static bool IsFileFiltered(const std::wstring&);

	static bool OverlaysEnabled();
	
	// Traverses DLL heap boundary, cannot work without ref
	static bool ReceiveResponse(const std::wstring&, std::wstring&);
};
