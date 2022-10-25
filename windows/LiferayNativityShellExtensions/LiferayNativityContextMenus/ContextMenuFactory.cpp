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

IFACEMETHODIMP ContextMenuFactory::CreateInstance(
    IUnknown* pUnkOuter, REFIID riid, void** ppv)
{
	if (pUnkOuter != NULL) {
		return CLASS_E_NOAGGREGATION;
	}
	
	auto liferayNativityContextMenus{ winrt::make<LiferayNativityContextMenus>() };
	if (!liferayNativityContextMenus)
	{
		return E_FAIL;
	}

	return liferayNativityContextMenus.as(riid, ppv);
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