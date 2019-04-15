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

#include "LiferayNativityOverlay.h"
#include <memory>

using namespace std;

#pragma comment(lib, "shlwapi.lib")

extern HINSTANCE instanceHandle;

#define IDM_DISPLAY 0
#define IDB_OK 101

struct HKEY_Deleter
{
	void operator()(HKEY hkey)
	{
		::RegCloseKey(hkey);
	}
};

typedef std::unique_ptr<HKEY__, HKEY_Deleter> HKEYPTR;

HKEYPTR FindImplementation(const wchar_t* filePath) {
	HKEY hkey;
	// open root key.
	if (FAILED(RegOpenKeyEx(HKEY_CURRENT_USER, REGISTRY_ROOT_KEY, NULL, KEY_READ, &hkey))) {
		return false;
	}
	HKEYPTR rootKey(hkey);

	DWORD subkeyCount;
	DWORD subkeyMaxLen;

	// Query for subkey length
	if (FAILED(
		RegQueryInfoKey(
			/* hKey */ rootKey.get(),
			/* lpClass */ NULL,
			/* lpcchClass*/ NULL,
			/* lpReserved */ NULL,
			/* lpcSubKeys */ &subkeyCount,
			/* lpcbMaxSubKeyLen */ &subkeyMaxLen,
			/* lpcbMaxClassLen */ NULL,
			/* lpcValues */ NULL,
			/* lpcbMaxValueNameLen */ NULL,
			/* lpcbMaxValueLen */ NULL,
			/* lpcbSecurityDescriptor */ NULL,
			/* lpftLastWriteTime */ NULL))) {
		return false;
	}

	HKEYPTR subKey;
	unique_ptr<CHAR[]> name(new char[subkeyMaxLen]);
	wchar_t value[4096];
	DWORD value_length = 4096;
	// open each and every one of them
	for (int i = 0; i < subkeyCount; i++)
	{
		// try get name
		if (FAILED(
			RegEnumKeyExA(
				/* hKey */ rootKey.get(),
				/* dwIndex */ i,
				/* lpName */ name.get(),
				/* lpcchName */ &subkeyMaxLen,
				/* lpReserved */ NULL,
				/* lpClass */ NULL,
				/* lpcchClass */ NULL,
				/* lpftLastWriteTime */ NULL))) {
			continue;
		}
		// open subkey
		if (FAILED(
			RegOpenKeyEx(
				/* hKey */ rootKey.get(),
				/* lpSubKey */ (LPCWSTR)name.get(),
				/* ulOptions */ NULL,
				/* samDesired */ KEY_READ,
				/* phkResult */ &hkey))) {
			continue;
		}
		subKey = HKEYPTR(hkey);

		// read filter folders
		// this performs json parsing early.
		if (FAILED(
			RegQueryValueEx(
				rootKey.get(),
				(LPCWSTR)REGISTRY_FILTER_FOLDERS,
				NULL,
				NULL,
				(LPBYTE)value,
				&value_length))) {
			continue;
		}

		Json::Reader jsonReader;
		Json::Value jsonFilterFolders;

		// not parsed correctly. skip.
		if (!jsonReader.parse(StringUtil::toString(value), jsonFilterFolders))
		{
			continue;
		}

		// ignore empty filter folders
		if (jsonFilterFolders.size() == 0) {
			continue;
		}

		for (unsigned int i = 0; i < jsonFilterFolders.size(); i++)
		{
			auto string = jsonFilterFolders[i].asString();
			if (FileUtil::IsChildFile(StringUtil::toWstring(string).c_str(), filePath))
			{
				// it is a child.
				// use it.
				subKey = HKEYPTR(hkey);
				break;
			}
		}
	}
	// this will be uninitialized if not set.
	return subKey;
}

LiferayNativityOverlay::LiferayNativityOverlay() : _referenceCount(1)
{
}

IFACEMETHODIMP_(ULONG) LiferayNativityOverlay::AddRef()
{
	return InterlockedIncrement(&_referenceCount);
}

IFACEMETHODIMP LiferayNativityOverlay::QueryInterface(REFIID riid, void** ppv)
{
	HRESULT hResult = S_OK;

	if (IsEqualIID(IID_IUnknown, riid) || IsEqualIID(IID_IShellIconOverlayIdentifier, riid))
	{
		*ppv = static_cast<IShellIconOverlayIdentifier*>(this);
	}
	else
	{
		hResult = E_NOINTERFACE;
		*ppv = NULL;
	}

	if (*ppv)
	{
		AddRef();
	}

	return hResult;
}

IFACEMETHODIMP_(ULONG) LiferayNativityOverlay::Release()
{
	ULONG cRef = InterlockedDecrement(&_referenceCount);
	if (0 == cRef)
	{
		delete this;
	}

	return cRef;
}

IFACEMETHODIMP LiferayNativityOverlay::GetPriority(int* pPriority)
{
	pPriority = 0;

	return S_OK;
}

IFACEMETHODIMP LiferayNativityOverlay::IsMemberOf(PCWSTR pwszPath, DWORD dwAttrib)
{
	HKEYPTR hkey = FindImplementation(pwszPath);
	if (!hkey) {
		return MAKE_HRESULT(S_FALSE, 0, 0);
	}

	if (!_IsOverlaysEnabled(hkey.get()))
	{
		return MAKE_HRESULT(S_FALSE, 0, 0);
	}

	// FindImplementation does this already …
	if (!FileUtil::IsFileFiltered(hkey.get(), pwszPath))
	{
		return MAKE_HRESULT(S_FALSE, 0, 0);
	}

	CommunicationSocket socket(hkey.get());
	if (!_IsMonitoredFileState(socket, pwszPath))
	{
		return MAKE_HRESULT(S_FALSE, 0, 0);
	}

	return MAKE_HRESULT(S_OK, 0, 0);
}

IFACEMETHODIMP LiferayNativityOverlay::GetOverlayInfo(PWSTR pwszIconFile, int cchMax, int* pIndex, DWORD* pdwFlags)
{
	*pIndex = 0;

	*pdwFlags = ISIOI_ICONFILE | ISIOI_ICONINDEX;

	if (GetModuleFileName(instanceHandle, pwszIconFile, cchMax) == 0)
	{
		HRESULT hResult = HRESULT_FROM_WIN32(GetLastError());

		return hResult;
	}

	return S_OK;
}

bool LiferayNativityOverlay::_IsOverlaysEnabled(HKEY hkey)
{
	wchar_t value[4096];
	DWORD value_length = 4096;
	if (FAILED(
		RegQueryValueEx(
			hkey, REGISTRY_ENABLE_OVERLAY, NULL, NULL, (LPBYTE)value, &value_length))) {
		return false;
	}

	return stoi((wstring)value);
}

bool LiferayNativityOverlay::_IsMonitoredFileState(CommunicationSocket& socket, const wchar_t* filePath)
{
	bool needed = false;

	Json::Value jsonRoot;

	jsonRoot[NATIVITY_COMMAND] = NATIVITY_GET_FILE_ICON_ID;
	jsonRoot[NATIVITY_VALUE] = StringUtil::toString(filePath);

	Json::FastWriter jsonWriter;

	wstring* message = new wstring();

	message->append(StringUtil::toWstring(jsonWriter.write(jsonRoot)));

	wstring* response = new wstring();

	if (!socket.SendMessageReceiveResponse(message->c_str(), response))
	{
		delete message;
		delete response;

		return false;
	}

	Json::Reader jsonReader;
	Json::Value jsonResponse;

	if (!jsonReader.parse(StringUtil::toString(*response), jsonResponse))
	{
		delete message;
		delete response;

		return false;
	}

	Json::Value jsonValue = jsonResponse.get(NATIVITY_VALUE, "");

	wstring valueString = StringUtil::toWstring(jsonValue.asString());

	if (valueString.size() == 0)
	{
		delete message;
		delete response;

		return false;
	}

	int state = _wtoi(valueString.c_str());

	if (state == OVERLAY_ID)
	{
		needed = true;
	}

	delete message;
	delete response;

	return needed;
}
