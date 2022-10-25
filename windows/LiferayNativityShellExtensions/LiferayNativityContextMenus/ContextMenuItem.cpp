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
#include "ContextMenuItem.h"

using namespace std;

ContextMenuItem::ContextMenuItem() : _enabled(false), _id(0)
{
}

const vector<ContextMenuItem*> ContextMenuItem::GetContextMenuItems()
{
	vector<ContextMenuItem*> result;
	for (auto& it : _contextMenuItems) {
		result.push_back(it.get());
	}
	return result;
}

bool ContextMenuItem::GetEnabled()
{
	return _enabled;
}

const wstring ContextMenuItem::GetHelpText()
{
	return _helpText;
}

const wstring ContextMenuItem::GetIconPath()
{
	return _iconPath;
}

long ContextMenuItem::GetId()
{
	return _id;
}

int ContextMenuItem::GetIndex()
{
	return _index;
}

const wstring ContextMenuItem::GetTitle()
{
	return _title;
}

const wstring ContextMenuItem::GetUuid()
{
	return _uuid;
}

bool ContextMenuItem::HasSubMenus()
{
	return _contextMenuItems.size() != 0;
}

void ContextMenuItem::SetContextMenuItems(vector<unique_ptr<ContextMenuItem>>&& list)
{
	_contextMenuItems = move(list);
}

void ContextMenuItem::SetEnabled(bool enabled)
{
	_enabled = enabled;
}

void ContextMenuItem::SetHelpText(const std::wstring& const helpText)
{
	_helpText = helpText;
}

void ContextMenuItem::SetIconPath(const std::wstring& const iconPath)
{
	_iconPath = iconPath;
}

void ContextMenuItem::SetId(long id)
{
	_id = id;
}

void ContextMenuItem::SetIndex(int index)
{
	_index = index;
}

void ContextMenuItem::SetTitle(const std::wstring& const title)
{
	_title = title;
}

void ContextMenuItem::SetUuid(const std::wstring& const uuid)
{
	_uuid = uuid;
}
