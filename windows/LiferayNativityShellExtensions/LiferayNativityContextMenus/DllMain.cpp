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

#include "ContextMenuFactory.h"
#include "ContextMenuConstants.h"
#include <Guiddef.h>

HINSTANCE instanceHandle = NULL;

long dllReferenceCount = 0;

BOOL APIENTRY DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved)
{
	switch (dwReason)
	{
		case DLL_PROCESS_ATTACH:
			instanceHandle = hModule;
			DisableThreadLibraryCalls(hModule);
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
	GUID guid;
	if (FAILED(CLSIDFromString(CONTEXT_MENU_GUID, (LPCLSID)&guid)) || !IsEqualCLSID(guid, rclsid))
	{
		return CLASS_E_CLASSNOTAVAILABLE;
	}

	auto contextMenuFactory{ winrt::make<ContextMenuFactory>() };
	if (!contextMenuFactory) {
		return E_FAIL;
	}
	return contextMenuFactory.as(riid, ppv);
}

STDAPI DllCanUnloadNow(void)
{
	return dllReferenceCount > 0 ? S_FALSE : S_OK;
}
