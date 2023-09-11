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

#include "ContextMenuAction.h"
#include "ContextMenuItem.h"
#include <json/json.h>
#include <NativityMessage.h>
#include <Windows.h>
#include <NativityUtil.h>

struct ContextMenuUtil
{
	bool AddFile(const std::wstring&);

	bool GetMenus(std::vector<ContextMenuItem*>&);

	bool GetContextMenuItem(int, ContextMenuItem*&);

	bool GetContextMenuAction(const std::wstring&, std::unique_ptr<ContextMenuAction>&);

	bool GetContextMenuAction(int action, std::unique_ptr<ContextMenuAction>&);

	bool InitMenus(void);

	bool PerformAction(int, HWND hWnd);

private:
	bool _GetContextMenuItem(int, const std::vector<ContextMenuItem*>&, ContextMenuItem*&);

	bool _ParseContextMenuList(std::wstring, std::vector<std::unique_ptr<ContextMenuItem>>);

	bool _ParseContextMenuItem(const Json::Value&, ContextMenuItem&);

	bool _ProcessContextMenus(NativityMessage);

	std::vector<std::unique_ptr<ContextMenuItem>> _menuList;

	std::vector<std::wstring> _selectedFiles;

	Nativity::Util::NativityUtil connection;
};
