#pragma once
#include <memory>
#include <string_view>
#include "API.h"

namespace Nativity::Util
{
	struct INativityUtil
	{
		virtual bool OverlaysEnabled() = 0;

		virtual bool ReceiveResponse(std::wstring_view const& Message, std::wstring& Response) = 0;
	};

	struct UTIL_API NativityUtil
	{
		NativityUtil(const NativityUtil&) = delete;
		NativityUtil& operator=(const NativityUtil&) = delete;
		void* operator new(size_t) = delete;

		NativityUtil() = default;
		NativityUtil(NativityUtil&&) = default;
		NativityUtil& operator=(NativityUtil&&) = default;

		NativityUtil(std::unique_ptr<INativityUtil>&& ptr);
		NativityUtil& operator=(std::unique_ptr<INativityUtil>&& ptr);

		static bool Find(std::wstring_view const& path, NativityUtil& util);

		INativityUtil* operator->();
		const INativityUtil* operator->() const;

		INativityUtil& operator*();
		const INativityUtil& operator*() const;

		operator bool() const;

	private:
		std::unique_ptr<INativityUtil> m_ptr{};
	};
}
