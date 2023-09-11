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

#include <memory>
#include <string>
#include <vector>

struct ContextMenuItem
{
	using ContextMenuItemList = std::vector<std::unique_ptr<ContextMenuItem>>;

	ContextMenuItem(void);

	void AddContextMenuItem(const ContextMenuItem&);

	const std::vector<ContextMenuItem*> GetContextMenuItems();

	bool GetEnabled();

	const std::wstring GetHelpText();

	const std::wstring GetIconPath();

	long GetId();

	int GetIndex();

	const std::wstring GetTitle();

	const std::wstring GetUuid();

	bool HasSubMenus();

	void SetContextMenuItems(ContextMenuItemList&&);

	void SetEnabled(bool);

	void SetHelpText(const std::wstring&);

	void SetIconPath(const std::wstring&);

	void SetId(long);

	void SetIndex(int);

	void SetTitle(const std::wstring&);

	void SetUuid(const std::wstring&);

private:
	ContextMenuItemList _contextMenuItems;

	bool _enabled;

	std::wstring _helpText;

	std::wstring _iconPath;

	long _id;

	int _index;

	std::wstring _title;

	std::wstring _uuid;
};
