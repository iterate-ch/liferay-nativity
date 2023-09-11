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
#include "API.h"
#include <string>

class UTIL_API NativityMessage
{
	public:
		NativityMessage(void) = default;

		std::wstring GetCommand();

		std::wstring GetValue();

		void SetCommand(std::wstring const&);

		void SetValue(std::wstring const&);

	private:
		std::wstring _command;
		std::wstring _value;
};
