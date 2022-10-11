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

#include "stdafx.h"
#include "LiferayNativityOverlay.h"
#include <memory>
#include <NativityUtil.h>
#include <StringUtil.h>
#include <json/json.h>
#include "UtilConstants.h"
#include "OverlayConstants.h"

using namespace std;

extern HINSTANCE instanceHandle;

#define IDM_DISPLAY 0
#define IDB_OK 101

IFACEMETHODIMP LiferayNativityOverlay::GetPriority(int* pPriority)
{
	pPriority = 0;

	return S_OK;
}

IFACEMETHODIMP LiferayNativityOverlay::IsMemberOf(PCWSTR pwszPath, DWORD dwAttrib)
{
	if (!_IsOverlaysEnabled())
	{
		return MAKE_HRESULT(S_FALSE, 0, 0);
	}

	if (!NativityUtil::IsFileFiltered(pwszPath)) {
		return MAKE_HRESULT(S_FALSE, 0, 0);
	}

	if (!_IsMonitoredFileState(pwszPath))
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

bool LiferayNativityOverlay::_IsOverlaysEnabled()
{
	return NativityUtil::OverlaysEnabled();
}

bool LiferayNativityOverlay::_IsMonitoredFileState(const wchar_t* filePath)
{
	bool needed = false;

	Json::Value jsonRoot;

	jsonRoot[NATIVITY_COMMAND] = NATIVITY_GET_FILE_ICON_ID;
	jsonRoot[NATIVITY_VALUE] = StringUtil::toString(filePath);

	Json::FastWriter jsonWriter;

	wstring message = StringUtil::toWstring(jsonWriter.write(jsonRoot));
	wstring response;
	if (!NativityUtil::ReceiveResponse(message, response)) {
		return false;
	}
	
	Json::Reader jsonReader;
	Json::Value jsonResponse;

	if (!jsonReader.parse(StringUtil::toString(response), jsonResponse))
	{
		return false;
	}

	Json::Value jsonValue = jsonResponse.get(NATIVITY_VALUE, "");

	wstring valueString = StringUtil::toWstring(jsonValue.asCString());

	if (valueString.size() == 0)
	{
		return false;
	}

	int state = _wtoi(valueString.c_str());

	if (state == OVERLAY_ID)
	{
		needed = true;
	}

	return needed;
}
