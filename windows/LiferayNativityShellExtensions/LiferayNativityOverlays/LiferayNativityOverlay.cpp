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
#include "UtilConstants.h"
#include "OverlayConstants.h"
#include <json/json.h>
#include <StringUtil.h>
#include <memory>

using namespace std;
using namespace Nativity::Util;

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
	NativityUtil connection;
	if (!NativityUtil::Find(pwszPath, connection)) {
		return MAKE_HRESULT(S_FALSE, 0, 0);
	}

	if (!connection->OverlaysEnabled()) {
		return MAKE_HRESULT(S_FALSE, 0, 0);
	}

	if (!IsMoniteredFileState(connection, pwszPath)) {
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

bool LiferayNativityOverlay::IsMoniteredFileState(NativityUtil& connection, const wchar_t* filePath) {
	Json::Value jsonRoot;

	jsonRoot[NATIVITY_COMMAND] = NATIVITY_GET_FILE_ICON_ID;
	jsonRoot[NATIVITY_VALUE] = StringUtil::toString(filePath);

	Json::FastWriter jsonWriter;

	wstring response;
	if (!connection->ReceiveResponse(StringUtil::toWstring(jsonWriter.write(jsonRoot)), response)) {
		return false;
	}

	Json::Reader jsonReader;
	Json::Value jsonResponse;

	if (!jsonReader.parse(StringUtil::toString(response), jsonResponse))
	{
		return false;
	}

	Json::Value jsonValue = jsonResponse.get(NATIVITY_VALUE, "");

	string valueString = jsonValue.asString();

	if (valueString.length() == 0)
	{
		return false;
	}

	int state = atoi(valueString.c_str());

	return state == OVERLAY_ID;
}
