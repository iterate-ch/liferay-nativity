/**
 * Copyright (c) 2000-2013 Liferay, Inc. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2.1 of the License, or (at your option)
 * any later version.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
 * details.
 */

#include "NativityOverlayRegistrationHandler.h"
#include "NativityOverlayFactory.h"
#include "stdafx.h"
#include <PathCch.h>
#include <string>
#include <sstream>
#include <filesystem>

#pragma comment(lib, "Pathcch.lib")

HINSTANCE instanceHandle = NULL;
std::wstring logFilePath;

long dllReferenceCount = 0;

void CreateLogFile(HMODULE hModule)
{
	PWSTR desktopPathName;
	SHGetKnownFolderPath(FOLDERID_Desktop, 0, NULL, &desktopPathName);
	std::experimental::filesystem::path desktopPath(desktopPathName);
	CoTaskMemFree(desktopPathName);

	std::vector<wchar_t> filename = std::vector<wchar_t>();
	filename.resize(MAX_PATH);
	while (!GetModuleFileName(hModule, filename.data(), filename.size()) || GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
		filename.resize(filename.size() * 2);
	}
	if (SUCCEEDED(GetLastError())) {
		std::experimental::filesystem::path filenamePath(filename.data());
		filenamePath = filenamePath.replace_extension(L"log");
		auto logFile{ desktopPath.append(filenamePath.filename()) };
		logFilePath = logFile.wstring();
	}
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved)
{
	switch (dwReason)
	{
	case DLL_PROCESS_ATTACH:
		instanceHandle = hModule;
		DisableThreadLibraryCalls(hModule);
		CreateLogFile(hModule);
		break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}

	return TRUE;
}

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, void** ppv)
{
	HRESULT hResult = CLASS_E_CLASSNOTAVAILABLE;
	GUID guid;

	hResult = CLSIDFromString(OVERLAY_GUID, (LPCLSID)&guid);

	if (hResult != S_OK)
	{
		return hResult;
	}

	if (!IsEqualCLSID(guid, rclsid))
	{
		return hResult;
	}

	hResult = E_OUTOFMEMORY;

	wchar_t szModule[MAX_PATH];

	if (GetModuleFileName(instanceHandle, szModule, ARRAYSIZE(szModule)) == 0)
	{
		hResult = HRESULT_FROM_WIN32(GetLastError());

		return hResult;
	}

	NativityOverlayFactory* nativityOverlayFactory = new NativityOverlayFactory(szModule);

	if (nativityOverlayFactory)
	{
		hResult = nativityOverlayFactory->QueryInterface(riid, ppv);
		nativityOverlayFactory->Release();
	}
	return hResult;
}

STDAPI DllCanUnloadNow(void)
{
	return dllReferenceCount > 0 ? S_FALSE : S_OK;

	return S_FALSE;
}

HRESULT _stdcall DllRegisterServer(void)
{
	HRESULT hResult = S_OK;

	wchar_t szModule[MAX_PATH];

	if (GetModuleFileName(instanceHandle, szModule, ARRAYSIZE(szModule)) == 0)
	{
		hResult = HRESULT_FROM_WIN32(GetLastError());

		return hResult;
	}

	GUID guid;

	hResult = CLSIDFromString(OVERLAY_GUID, (LPCLSID)&guid);

	if (hResult != S_OK)
	{
		return hResult;
	}

	hResult = NativityOverlayRegistrationHandler::RegisterCOMObject(szModule, guid);

	if (FAILED(hResult))
	{
		return hResult;
	}

	hResult = NativityOverlayRegistrationHandler::MakeRegistryEntries(guid, OVERLAY_NAME);

	if (FAILED(hResult))
	{
		return hResult;
	}

	return hResult;
}

STDAPI DllUnregisterServer(void)
{
	HRESULT hResult = S_OK;

	wchar_t szModule[MAX_PATH];

	if (GetModuleFileNameW(instanceHandle, szModule, ARRAYSIZE(szModule)) == 0)
	{
		hResult = HRESULT_FROM_WIN32(GetLastError());
		return hResult;
	}

	GUID guid;

	hResult = CLSIDFromString(OVERLAY_GUID, (LPCLSID)&guid);

	if (hResult != S_OK)
	{
		return hResult;
	}

	hResult = NativityOverlayRegistrationHandler::UnregisterCOMObject(guid);

	if (FAILED(hResult))
	{
		return hResult;
	}

	hResult = NativityOverlayRegistrationHandler::RemoveRegistryEntries(OVERLAY_NAME);

	if (FAILED(hResult))
	{
		return hResult;
	}

	return hResult;
}
