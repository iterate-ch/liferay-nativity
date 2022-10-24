/**
 *  Copyright (c) 2000-2013 Liferay, Inc. All rights reserved.
 *
 *  This library is free software; you can redistribute it and/or modify it under
 *  the terms of the GNU Lesser General Public License as published by the Free
 *  Software Foundation; either version 2.1 of the License, or (at your option)
 *  any later version.
 *
 *  This library is distributed in the hope that it will be useful, but WITHOUT
 *  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 *  FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
 *  details.
 */

#include "StringUtil.h"
#include <comutil.h>
#include <stdexcept>
#include <Windows.h>

using namespace std;

// Implementation: Curtesy of https://stackoverflow.com/a/69410299/1417828

wstring StringUtil::toWstring(const string_view& str)
{
	if (str.empty()) {
		return L"";
	}

	const auto size_needed = MultiByteToWideChar(CP_UTF8, 0, &str.at(0), (int)str.size(), nullptr, 0);
	if (size_needed <= 0)
	{
		throw std::runtime_error("MultiByteToWideChar() failed: " + std::to_string(size_needed));
	}

	std::wstring result(size_needed, NULL);
	MultiByteToWideChar(CP_UTF8, 0, &str.at(0), (int)str.size(), result.data(), size_needed);
	return result;
}

string StringUtil::toString(const wstring_view& wstr)
{
	if (wstr.empty()) {
		return "";
	}

	const auto size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr.at(0), (int)wstr.size(), nullptr, 0, nullptr, nullptr);
	if (size_needed <= 0)
	{
		throw std::runtime_error("WideCharToMultiByte() failed: " + std::to_string(size_needed));
	}

	std::string result(size_needed, NULL);
	WideCharToMultiByte(CP_UTF8, 0, &wstr.at(0), (int)wstr.size(), result.data(), size_needed, nullptr, nullptr);
	return result;
}

