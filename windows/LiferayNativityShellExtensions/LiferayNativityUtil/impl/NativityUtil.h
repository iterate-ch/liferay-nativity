#pragma once
#include "../NativityUtil.h"
#include <INativity.h>
#include <mutex>
#include <winrt/base.h>

namespace Nativity::Util::implementation
{
	struct NativityUtil : INativityUtil
	{
		using class_type = Nativity::Util::NativityUtil;

		NativityUtil();
		NativityUtil(winrt::com_ptr<INativity>& connection);

		static bool Find(std::wstring_view const&, class_type&);

		bool OverlaysEnabled();

		bool ReceiveResponse(std::wstring_view const&, std::wstring&);

	private:
		winrt::com_ptr<INativity> connection;
	};
}