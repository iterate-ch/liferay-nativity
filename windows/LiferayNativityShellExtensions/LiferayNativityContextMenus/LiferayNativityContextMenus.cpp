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
#include "LiferayNativityContextMenus.h"
#include <shellapi.h>
#include "ContextMenuUtil.h"
#include "StringUtil.h"

LiferayNativityContextMenus::LiferayNativityContextMenus() : _contextMenuUtil(nullptr), _nFiles(0)
{
}

IFACEMETHODIMP LiferayNativityContextMenus::GetCommandString(UINT_PTR idCommand, UINT uFlags, UINT* pwReserved, LPSTR pszName, UINT cchMax)
{
	HRESULT hResult = S_OK;
	ContextMenuItem* item;

	switch (uFlags)
	{
	case GCS_HELPTEXTW:
		if (!_contextMenuUtil->GetContextMenuItem((int)idCommand, item))
		{
			return E_FAIL;
		}

		wcscpy_s((wchar_t*)pszName, cchMax, item->GetHelpText().c_str());

		break;

	case GCS_VERBW:
		if (!_contextMenuUtil->GetContextMenuItem((int)idCommand, item))
		{
			return E_FAIL;
		}

		_itow_s(item->GetId(), (wchar_t*)pszName, cchMax, 10);

		break;

	default:
		hResult = S_OK;
	}

	return hResult;
}

IFACEMETHODIMP LiferayNativityContextMenus::Initialize(LPCITEMIDLIST pidlFolder, LPDATAOBJECT pDataObj, HKEY hKeyProgID)
{
	wstring szFileName(MAX_PATH, L'\0');

	if (pDataObj)
	{
		FORMATETC fe = { CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };

		STGMEDIUM stm;

		if (FAILED(pDataObj->GetData(&fe, &stm)))
		{
			return E_INVALIDARG;
		}

		HDROP hDrop = static_cast<HDROP>(GlobalLock(stm.hGlobal));

		if (hDrop == NULL)
		{
			return E_INVALIDARG;
		}

		_nFiles = DragQueryFile(hDrop, 0xFFFFFFFF, NULL, 0);

		if (_nFiles > 0)
		{
			_contextMenuUtil = make_unique<ContextMenuUtil>();

			for (UINT i = 0; i < _nFiles; i++)
			{
				UINT success = DragQueryFile(hDrop, i, szFileName.data(), szFileName.capacity());

				if (success != 0)
				{
					szFileName.resize(wcslen(szFileName.c_str()));
					_contextMenuUtil->AddFile(szFileName);
				}
			}
		}

		GlobalUnlock(stm.hGlobal);

		ReleaseStgMedium(&stm);
	}
	else if (pidlFolder)
	{
		if (!SHGetPathFromIDList(pidlFolder, szFileName.data()))
		{
			return E_INVALIDARG;
		}
		szFileName.resize(wcslen(szFileName.c_str()));

		_nFiles = 1;

		_contextMenuUtil = make_unique<ContextMenuUtil>();

		_contextMenuUtil->AddFile(szFileName);
	}

	return S_OK;
}

IFACEMETHODIMP LiferayNativityContextMenus::InvokeCommand(LPCMINVOKECOMMANDINFO pici)
{
	if (!_contextMenuUtil)
	{
		_contextMenuUtil = make_unique<ContextMenuUtil>();
	}

	bool unicode = false;
	int index = -1;

	if (pici->cbSize == sizeof(CMINVOKECOMMANDINFOEX))
	{
		if (pici->fMask & CMIC_MASK_UNICODE)
		{
			unicode = true;
		}
	}

	if (!unicode && HIWORD(pici->lpVerb))
	{
		index = atoi(pici->lpVerb);

	}
	else if (unicode && HIWORD(((CMINVOKECOMMANDINFOEX*)pici)->lpVerbW))
	{
		index = _wtoi(((CMINVOKECOMMANDINFOEX*)pici)->lpVerbW);
	}
	else
	{
		index = LOWORD(pici->lpVerb);
	}

	if (index < 0)
	{
		return E_FAIL;
	}

	if (!_contextMenuUtil->PerformAction(index, pici->hwnd))
	{
		return E_FAIL;
	}
	return S_OK;
}

IFACEMETHODIMP LiferayNativityContextMenus::QueryContextMenu(HMENU hMenu, UINT indexMenu, UINT idCmdFirst, UINT idCmdLast, UINT uFlags)
{
	//No need for menu, this should not happen
	if (_nFiles == 0)
	{
		return MAKE_HRESULT(SEVERITY_SUCCESS, 0, USHORT(0));
	}

	//Only default menus
	if (CMF_DEFAULTONLY & uFlags)
	{
		return MAKE_HRESULT(SEVERITY_SUCCESS, 0, USHORT(0));
	}

	if (!_contextMenuUtil->IsMenuNeeded())
	{
		return MAKE_HRESULT(SEVERITY_SUCCESS, 0, USHORT(0));
	}

	if (!_contextMenuUtil->InitMenus())
	{
		return MAKE_HRESULT(SEVERITY_SUCCESS, 0, USHORT(0));
	}

	vector<ContextMenuItem*> menus;

	if (!_contextMenuUtil->GetMenus(menus))
	{
		return MAKE_HRESULT(SEVERITY_SUCCESS, 0, USHORT(0));
	}

	//No menus for these files
	if ((menus.size() == 0) || menus.empty())
	{
		return MAKE_HRESULT(SEVERITY_SUCCESS, 0, USHORT(0));
	}

	bool success = true;

	int location = indexMenu;

	int cmdCount = idCmdFirst;

	_InsertSeparator(hMenu, location);

	location++;

	cmdCount++;

	for (auto it : menus) {
		cmdCount = _AddMenu(hMenu, *it, location, cmdCount, idCmdFirst);

		location++;
	}

	_InsertSeparator(hMenu, location);

	cmdCount++;

	return MAKE_HRESULT(SEVERITY_SUCCESS, 0, cmdCount - idCmdFirst + 1);
}

int LiferayNativityContextMenus::_AddMenu(HMENU hMenu, ContextMenuItem& menu, int location, int cmdCount, UINT offset)
{
	wstring text = menu.GetTitle();

	menu.SetIndex(cmdCount - offset);

	if (text.compare(SEPARATOR) == 0)
	{
		if (_InsertSeparator(hMenu, location))
		{
			cmdCount++;
		}
	}
	else {
		if (menu.HasSubMenus())
		{
			HMENU subMenuHandle = CreatePopupMenu();

			if (_InsertMenu(hMenu, subMenuHandle, location, text, menu.GetEnabled()))
			{
				cmdCount++;

				int subLocation = 0;

				auto menus{ menu.GetContextMenuItems() };

				for (auto it : menus) {
					cmdCount = _AddMenu(subMenuHandle, *it, subLocation, cmdCount, offset);
					subLocation++;
				}
			}
		}
		else
		{
			if (_InsertMenu(hMenu, location, cmdCount, text.c_str(), menu.GetEnabled()))
			{
				cmdCount++;
			}
		}

		SIZE sizIcon;
		sizIcon.cx = GetSystemMetrics(SM_CXSMICON);
		sizIcon.cy = GetSystemMetrics(SM_CYSMICON);
		HICON hIcon = (HICON)LoadImage(NULL, menu.GetIconPath().c_str(), IMAGE_ICON, sizIcon.cx, sizIcon.cy, LR_LOADFROMFILE);

		if (hIcon != NULL)
		{
			HBITMAP bitmap = _IconToBitmapPARGB32(hIcon);

			SetMenuItemBitmaps(hMenu, location, MF_BYPOSITION, bitmap, bitmap);
		}
	}

	return cmdCount;
}

HRESULT LiferayNativityContextMenus::_ConvertBufferToPARGB32(HPAINTBUFFER hPaintBuffer, HDC hdc, HICON hicon, SIZE& sizIcon)
{
	RGBQUAD* prgbQuad;
	int cxRow;
	HRESULT hResult = GetBufferedPaintBits(hPaintBuffer, &prgbQuad, &cxRow);

	if (SUCCEEDED(hResult))
	{
		Gdiplus::ARGB* pargb = reinterpret_cast<Gdiplus::ARGB*>(prgbQuad);

		if (!_HasAlpha(pargb, sizIcon, cxRow))
		{
			ICONINFO info;

			if (GetIconInfo(hicon, &info))
			{

				if (info.hbmMask)
				{
					hResult = _ConvertToPARGB32(hdc, pargb, info.hbmMask, sizIcon, cxRow);
				}

				DeleteObject(info.hbmColor);
				DeleteObject(info.hbmMask);
			}
		}
	}

	return hResult;
}

HRESULT LiferayNativityContextMenus::_ConvertToPARGB32(HDC hdc, __inout Gdiplus::ARGB* pargb, HBITMAP hbmp, SIZE& sizImage, int cxRow)
{
	BITMAPINFO bmi;
	SecureZeroMemory(&bmi, sizeof(bmi));
	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biCompression = BI_RGB;
	bmi.bmiHeader.biWidth = sizImage.cx;
	bmi.bmiHeader.biHeight = sizImage.cy;
	bmi.bmiHeader.biBitCount = 32;

	HANDLE hHeap = GetProcessHeap();

	void* pvBits = HeapAlloc(hHeap, 0, bmi.bmiHeader.biWidth * 4 * bmi.bmiHeader.biHeight);

	if (pvBits == 0)
	{
		return E_OUTOFMEMORY;
	}

	HRESULT hResult = E_UNEXPECTED;

	if (GetDIBits(hdc, hbmp, 0, bmi.bmiHeader.biHeight, pvBits, &bmi, DIB_RGB_COLORS) == bmi.bmiHeader.biHeight)
	{
		ULONG cxDelta = cxRow - bmi.bmiHeader.biWidth;
		Gdiplus::ARGB* pargbMask = static_cast<Gdiplus::ARGB*>(pvBits);

		for (ULONG y = bmi.bmiHeader.biHeight; y; --y)
		{
			for (ULONG x = bmi.bmiHeader.biWidth; x; --x)
			{
				if (*pargbMask++)
				{
					// transparent pixel
					*pargb++ = 0;
				}
				else
				{
					// opaque pixel
					*pargb++ |= 0xFF000000;
				}
			}
			pargb += cxDelta;
		}

		hResult = S_OK;
	}

	HeapFree(hHeap, 0, pvBits);

	return hResult;
}

HRESULT LiferayNativityContextMenus::_Create32BitHBITMAP(HDC hdc, const SIZE* psize, __deref_opt_out void** ppvBits, __out HBITMAP* phBmp)
{
	if (psize == 0)
		return E_INVALIDARG;

	if (phBmp == 0)
		return E_POINTER;

	*phBmp = NULL;

	BITMAPINFO bmi;
	SecureZeroMemory(&bmi, sizeof(bmi));
	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biCompression = BI_RGB;
	bmi.bmiHeader.biWidth = psize->cx;
	bmi.bmiHeader.biHeight = psize->cy;
	bmi.bmiHeader.biBitCount = 32;

	HDC hdcUsed = hdc ? hdc : GetDC(NULL);

	if (hdcUsed)
	{
		*phBmp = CreateDIBSection(hdcUsed, &bmi, DIB_RGB_COLORS, ppvBits, NULL, 0);

		if (hdc != hdcUsed)
		{
			ReleaseDC(NULL, hdcUsed);
		}
	}

	return (NULL == *phBmp) ? E_OUTOFMEMORY : S_OK;
}

bool LiferayNativityContextMenus::_HasAlpha(__in Gdiplus::ARGB* pargb, SIZE& sizImage, int cxRow)
{
	ULONG cxDelta = cxRow - sizImage.cx;

	for (ULONG y = sizImage.cy; y; --y)
	{
		for (ULONG x = sizImage.cx; x; --x)
		{
			if (*pargb++ & 0xFF000000)
			{
				return true;
			}
		}

		pargb += cxDelta;
	}

	return false;
}

HBITMAP LiferayNativityContextMenus::_IconToBitmapPARGB32(HICON hIcon)
{
	if (!hIcon)
		return NULL;

	SIZE sizIcon;
	sizIcon.cx = GetSystemMetrics(SM_CXSMICON);
	sizIcon.cy = GetSystemMetrics(SM_CYSMICON);

	RECT rcIcon;
	SetRect(&rcIcon, 0, 0, sizIcon.cx, sizIcon.cy);
	HBITMAP hBmp = NULL;

	HDC hdcDest = CreateCompatibleDC(NULL);
	if (hdcDest)
	{
		if (SUCCEEDED(_Create32BitHBITMAP(hdcDest, &sizIcon, NULL, &hBmp)))
		{
			HBITMAP hbmpOld = (HBITMAP)SelectObject(hdcDest, hBmp);
			if (hbmpOld)
			{
				BLENDFUNCTION bfAlpha = { AC_SRC_OVER, 0, 255, AC_SRC_ALPHA };
				BP_PAINTPARAMS paintParams = { 0 };
				paintParams.cbSize = sizeof(paintParams);
				paintParams.dwFlags = BPPF_ERASE;
				paintParams.pBlendFunction = &bfAlpha;

				HDC hdcBuffer;
				HPAINTBUFFER hPaintBuffer = BeginBufferedPaint(hdcDest, &rcIcon, BPBF_DIB, &paintParams, &hdcBuffer);
				if (hPaintBuffer)
				{
					if (DrawIconEx(hdcBuffer, 0, 0, hIcon, sizIcon.cx, sizIcon.cy, 0, NULL, DI_NORMAL))
					{
						// If icon did not have an alpha channel we need to convert buffer to PARGB
						_ConvertBufferToPARGB32(hPaintBuffer, hdcDest, hIcon, sizIcon);
					}

					// This will write the buffer contents to the destination bitmap
					EndBufferedPaint(hPaintBuffer, TRUE);
				}

				SelectObject(hdcDest, hbmpOld);
			}
		}

		DeleteDC(hdcDest);
	}

	return hBmp;
}

bool LiferayNativityContextMenus::_InsertMenu(HMENU hMenu, HMENU subMenuHandle, int location, const wstring& const text, bool enabled)
{
	MENUITEMINFO menuItem = { sizeof(menuItem) };

	menuItem.fMask = MIIM_STRING | MIIM_SUBMENU | MIIM_STATE;

	menuItem.dwTypeData = (LPWSTR)text.c_str();

	menuItem.hSubMenu = subMenuHandle;

	menuItem.fState = enabled ? MFS_ENABLED : MFS_GRAYED;

	if (!InsertMenuItem(hMenu, location, TRUE, &menuItem))
	{
		return false;
	}

	return true;
}

bool LiferayNativityContextMenus::_InsertMenu(HMENU hMenu, int location, int command, const wstring& const text, bool enabled)
{
	MENUITEMINFO menuItem = { sizeof(menuItem) };

	menuItem.fMask = MIIM_STRING | MIIM_ID | MIIM_STATE;

	menuItem.dwTypeData = (LPWSTR)text.c_str();

	menuItem.wID = command;

	menuItem.fState = enabled ? MFS_ENABLED : MFS_GRAYED;

	if (!InsertMenuItem(hMenu, location, TRUE, &menuItem))
	{
		return false;
	}

	return true;
}

bool LiferayNativityContextMenus::_InsertSeparator(HMENU hMenu, int location)
{
	MENUITEMINFO sep = { sizeof(sep) };
	sep.fMask = MIIM_TYPE;
	sep.fType = MFT_SEPARATOR;

	if (!InsertMenuItem(hMenu, location, TRUE, &sep))
	{
		return false;
	}

	return true;
}