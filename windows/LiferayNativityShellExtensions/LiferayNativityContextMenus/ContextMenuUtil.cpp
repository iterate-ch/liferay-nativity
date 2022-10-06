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
#include "ContextMenuUtil.h"
#include <NativityUtil.h>
#include <memory>
#include <StringUtil.h>
#include <UtilConstants.h>

using namespace std;

bool ContextMenuUtil::AddFile(const wstring& const file)
{
	_selectedFiles.push_back(file);

	return true;
}

bool ContextMenuUtil::GetContextMenuItem(int index, ContextMenuItem*& item)
{
	vector<ContextMenuItem*> menuList;
	GetMenus(menuList);
	return _GetContextMenuItem(index, menuList, item);
}

bool ContextMenuUtil::_GetContextMenuItem(int index, const vector<ContextMenuItem*>& const menus, ContextMenuItem*& item)
{
	for (auto it : menus) {
		if (it->GetIndex() == index) {
			item = it;
			return true;
		}

		auto other{ it->GetContextMenuItems() };
		if (other.size() > 0) {
			if (_GetContextMenuItem(index, other, item)) {
				return true;
			}
		}
	}
	return false;
}

bool ContextMenuUtil::GetMenus(vector<ContextMenuItem*>& menuList)
{
	menuList = vector<ContextMenuItem*>(_menuList.size());
	for (auto& it : _menuList) {
		menuList.push_back(it.get());
	}
	return true;
}

bool ContextMenuUtil::IsMenuNeeded(void)
{
	for (auto selectedFile : _selectedFiles) {
		if (NativityUtil::IsFileFiltered(selectedFile)) {
			return true;
		}
	}
	return false;
}

bool ContextMenuUtil::InitMenus(void)
{
	Json::Value jsonRoot;

	jsonRoot[NATIVITY_COMMAND] = NATIVITY_GET_CONTEXT_MENU_LIST;

	for (auto it : _selectedFiles) {
		jsonRoot[NATIVITY_VALUE].append(StringUtil::toString(it));
	}

	Json::FastWriter jsonWriter;

	wstring getMenuMessage = StringUtil::toWstring(jsonWriter.write(jsonRoot));

	wstring getMenuReceived;
	if (NativityUtil::ReceiveResponse(getMenuMessage, getMenuReceived)) {
		Json::Reader jsonReader;
		Json::Value jsonResponse;

		if (!jsonReader.parse(StringUtil::toString(getMenuReceived), jsonResponse))
		{
			return false;
		}

		Json::Value jsonContextMenuItemsList = jsonResponse.get(NATIVITY_VALUE, "");

		for (unsigned int i = 0; i < jsonContextMenuItemsList.size(); i++)
		{
			Json::Value jsonContextMenuItem = jsonContextMenuItemsList[i];

			auto contextMenuItem{ make_unique<ContextMenuItem>() };
			contextMenuItem->SetId(i);

			if (!_ParseContextMenuItem(jsonContextMenuItem, *contextMenuItem))
			{
				return false;
			}
			_menuList.emplace_back(move(contextMenuItem));
		}
	}

	return true;
}

bool ContextMenuUtil::_ParseContextMenuItem(const Json::Value& jsonContextMenuItem, ContextMenuItem& const contextMenuItem)
{
	// enabled

	bool enabled = jsonContextMenuItem.get(NATIVITY_ENABLED, true).asBool();

	contextMenuItem.SetEnabled(enabled);

	// title

	auto title = StringUtil::toWstring(jsonContextMenuItem.get(NATIVITY_TITLE, "").asCString());

	if (title.size() == 0)
	{
		return false;
	}

	contextMenuItem.SetTitle(title);

	// uuid

	auto uuid = StringUtil::toWstring(jsonContextMenuItem.get(NATIVITY_UUID, "").asCString());

	if (uuid.size() == 0)
	{
		return false;
	}

	contextMenuItem.SetUuid(uuid);

	// help text

	auto helpText = StringUtil::toWstring(jsonContextMenuItem.get(NATIVITY_HELP_TEXT, "").asCString());

	contextMenuItem.SetHelpText(helpText);

	// icon path

	auto iconPath = StringUtil::toWstring(jsonContextMenuItem.get(NATIVITY_ICON_PATH, "").asCString());

	contextMenuItem.SetIconPath(iconPath);

	// children context menu items

	Json::Value jsonChildrenContextMenuItems = jsonContextMenuItem.get(NATIVITY_CONTEXT_MENU_ITEMS, "");

	vector<unique_ptr<ContextMenuItem>> childrenContextMenuItems;

	for (auto jsonChildContextMenuItem : jsonChildrenContextMenuItems) {
		auto childContextMenuItem{ make_unique<ContextMenuItem>() };

		_ParseContextMenuItem(jsonChildContextMenuItem, *childContextMenuItem);

		childrenContextMenuItems.emplace_back(move(childContextMenuItem));
	}

	contextMenuItem.SetContextMenuItems(move(childrenContextMenuItems));

	return true;
}

bool ContextMenuUtil::GetContextMenuAction(const std::wstring& const title, unique_ptr<ContextMenuAction>& item)
{
	for (auto& it : _menuList) {
		if (title.compare(it->GetTitle()) == 0) {
			item = make_unique<ContextMenuAction>();
			item->SetUuid(it->GetUuid());
			item->SetFiles(_selectedFiles);
			return true;
		}
	}

	return false;
}

bool ContextMenuUtil::GetContextMenuAction(int action, unique_ptr<ContextMenuAction>& const item)
{
	ContextMenuItem* contextMenuItem;

	if (GetContextMenuItem(action, contextMenuItem))
	{
		item = make_unique<ContextMenuAction>();
		item->SetUuid(contextMenuItem->GetUuid());
		item->SetFiles(_selectedFiles);

		return true;
	}

	return false;
}

bool ContextMenuUtil::PerformAction(int command, HWND hWnd)
{
	ContextMenuItem* contextMenuItem;

	if (!GetContextMenuItem(command, contextMenuItem))
	{
		return false;
	}

	Json::Value jsonValue;

	jsonValue[NATIVITY_UUID] = StringUtil::toString(contextMenuItem->GetUuid());
#ifdef _WIN64
	jsonValue[NATIVITY_HWND] = (int64_t)hWnd;
#else
	jsonValue[NATIVITY_HWND] = (int32_t)hWnd;
#endif

	for (auto it : _selectedFiles) {
		jsonValue[NATIVITY_FILES].append(StringUtil::toString(it));
	}

	Json::Value jsonRoot;

	jsonRoot[NATIVITY_COMMAND] = NATIVITY_CONTEXT_MENU_ACTION;
	jsonRoot[NATIVITY_VALUE] = jsonValue;

	Json::FastWriter jsonWriter;

	wstring jsonMessage = StringUtil::toWstring(jsonWriter.write(jsonRoot));
	wstring response;
	return NativityUtil::ReceiveResponse(jsonMessage, response);
}