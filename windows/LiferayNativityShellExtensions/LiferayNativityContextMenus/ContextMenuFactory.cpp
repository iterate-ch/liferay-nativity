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
#include "ContextMenuFactory.h"
#include "LiferayNativityContextMenus.h"

extern long dllReferenceCount;

ContextMenuFactory::ContextMenuFactory(wchar_t* modulePath) : _modulePath(modulePath)
{
}

ContextMenuFactory::~ContextMenuFactory()
{
}

IFACEMETHODIMP ContextMenuFactory::CreateInstance(
    IUnknown* pUnkOuter, REFIID riid, void** ppv)
{
	HRESULT hResult = CLASS_E_NOAGGREGATION;

	if (pUnkOuter != NULL)
	{
		return hResult;
	}

	hResult = E_OUTOFMEMORY;

	LiferayNativityContextMenus* liferayNativityContextMenus = new(std::nothrow) LiferayNativityContextMenus();

	if (!liferayNativityContextMenus)
	{
		return hResult;
	}

	hResult = liferayNativityContextMenus->QueryInterface(riid, ppv);

	liferayNativityContextMenus->Release();

	return hResult;
}

IFACEMETHODIMP ContextMenuFactory::LockServer(BOOL fLock)
{
	if (fLock)
	{
		InterlockedIncrement(&dllReferenceCount);
	}
	else
	{
		InterlockedDecrement(&dllReferenceCount);
	}
	return S_OK;
}