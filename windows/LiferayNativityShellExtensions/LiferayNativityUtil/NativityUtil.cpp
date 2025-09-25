#include "impl/NativityUtil.h"
#include "UtilConstants.h"
#include <functional>
#include <mutex>

using namespace std;
using namespace winrt;
using namespace winrt::impl;

namespace Nativity::Util::implementation
{
	NativityUtil::NativityUtil() = default;
	NativityUtil::NativityUtil(com_ptr<INativity>& connection) : connection(move(connection)) {};

	bool InstallProxy() {
		CLSID psclsid;
		if (SUCCEEDED(CoGetPSClsid(IID_INativity, &psclsid))) {
			return true;
		}

		if (!IsEqualCLSID(psclsid, CLSID_NULL)) {
			return true;
		}

		com_ptr<IPSFactoryBuffer> psFactoryBuffer;
		if (FAILED(DllGetClassObject(IID_INativity, IID_PPV_ARGS(psFactoryBuffer.put())))) {
			return false;
		}

		DWORD cookie;
		if (FAILED(CoRegisterClassObject(IID_INativity, psFactoryBuffer.get(), CLSCTX_INPROC_SERVER, REGCLS_MULTIPLEUSE, &cookie))) {
			return false;
		}

		if (FAILED(CoRegisterPSClsid(IID_INativity, IID_INativity))) {
			CoRevokeClassObject(cookie);
			return false;
		}

		return true;
	}

	static const com_ptr<IMoniker> const GetNativityClassMoniker() {
		static com_ptr<IMoniker> instance;
		atomic_thread_fence(memory_order_acquire);
		com_ptr local{ instance };
		if (!local) {
			static mutex initializerMutex;
			lock_guard initializerLock{ initializerMutex };
			atomic_thread_fence(memory_order_relaxed);
			local = instance;
			if (!local) {
				if (SUCCEEDED(CreateClassMoniker(IID_INativity, local.put()))) {
					atomic_thread_fence(memory_order_release);
					instance = local;
				}
			}
		}

		return local;
	}

	bool NativityUtil::Find(wstring_view const& path, class_type& util) {
		if (!InstallProxy()) {
			return false;
		}

		com_ptr classMoniker{ GetNativityClassMoniker() };
		if (!classMoniker) {
			return false;
		}

		com_ptr<IRunningObjectTable> rot;
		if (FAILED(GetRunningObjectTable(0, rot.put()))) {
			return false;
		}

		com_ptr<IEnumMoniker> enumMoniker;
		if (FAILED(rot->EnumRunning(enumMoniker.put()))) {
			return false;
		}

		com_ptr<IMoniker> moniker;
		while (enumMoniker->Next(1, moniker.put(), nullptr) != S_FALSE)
		{
			if (classMoniker->IsEqual(moniker.get()) != S_OK) {
				continue;
			}

			com_ptr<IUnknown> unkn;
			if (FAILED(rot->GetObject(moniker.get(), unkn.put()))) {
				continue;
			}

			com_ptr<INativity> nativity;
			if (!unkn.try_as(nativity)) {
				continue;
			}

			try {
				VARIANT_BOOL filter;
				if (FAILED(nativity->IsFiltered(path.data(), &filter))) {
					continue;
				}

				if (filter == VARIANT_TRUE) {
					util = make_unique<NativityUtil>(nativity);
					return true;
				}
			}
			catch (...) {}
		}

		return false;
	}

	bool NativityUtil::OverlaysEnabled() {
		VARIANT_BOOL result;
		if (FAILED(connection->OverlaysEnabled(&result))) {
			return false;
		}

		return result == VARIANT_TRUE;
	}

	bool NativityUtil::ReceiveResponse(wstring_view const& message, wstring& response) {
		bstr_handle response_local;
		if (FAILED(connection->ReceiveMessage(message.data(), response_local.put()))) {
			return false;
		}

		response.assign(response_local.get());
		return true;
	}
}

/* Default Implementation */
namespace Nativity::Util
{
	NativityUtil::NativityUtil(unique_ptr<INativityUtil>&& ptr) : m_ptr(std::move(ptr)) {}
	NativityUtil& NativityUtil::operator=(std::unique_ptr<INativityUtil>&& ptr) {
		if (m_ptr == ptr) {
			return *this;
		}

		m_ptr = std::move(ptr);
		return *this;
	}

	INativityUtil* NativityUtil::operator->() { return m_ptr.get(); }
	const INativityUtil* NativityUtil::operator->() const { return m_ptr.get(); };

	INativityUtil& NativityUtil::operator*() { return *m_ptr.get(); }
	const INativityUtil& NativityUtil::operator*() const { return *m_ptr.get(); }

	NativityUtil::operator bool() const {
		return m_ptr.operator bool();
	}
}

/* Simple forwarder to implementation. */
namespace Nativity::Util
{
	bool NativityUtil::Find(wstring_view const& path, NativityUtil& util) {
		return implementation::NativityUtil::Find(path, util);
	}
}
