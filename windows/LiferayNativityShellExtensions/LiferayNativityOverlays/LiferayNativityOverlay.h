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

#pragma once

#include <ShlObj.h>
#include <NativityUtil.h>
#include <winrt/base.h>

struct LiferayNativityOverlay : winrt::implements<LiferayNativityOverlay, IShellIconOverlayIdentifier>
{
	LiferayNativityOverlay() = default;

	IFACEMETHODIMP GetOverlayInfo(PWSTR pwszIconFile, int cchMax, int* pIndex, DWORD* pdwFlags);

	IFACEMETHODIMP GetPriority(int* pPriority);

	IFACEMETHODIMP IsMemberOf(PCWSTR pwszPath, DWORD dwAttrib);

private:
	static bool IsMoniteredFileState(Nativity::Util::NativityUtil& connection, const wchar_t* filePath);

	Nativity::Util::NativityUtil connection;
};
