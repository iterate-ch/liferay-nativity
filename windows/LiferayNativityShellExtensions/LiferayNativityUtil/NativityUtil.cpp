#include "NativityUtil.h"
#include <comdef.h>
#include <functional>
#include <winrt/base.h>
#include "UtilConstants.h"

#include "INativity.h"

class DECLSPEC_UUID("8FE0A7E5-25A4-45A3-9694-37ED25D333C7")
	ObjectProvider;
constexpr CLSID CLSID_ObjectProvider = __uuidof(ObjectProvider);
class DECLSPEC_UUID("3B512D20-7C95-44ED-A0A9-267C0AC9A428")
	Nativity;
constexpr CLSID CLSID_Nativity = __uuidof(Nativity);

_COM_SMARTPTR_TYPEDEF(INativity, IID_INativity);

using namespace winrt;
using namespace impl;

bool Connect(std::function<bool(INativityPtr)> Callback) {
	IBindCtxPtr bindCtx;
	check_hresult(CreateBindCtx(0, &bindCtx));
	IRunningObjectTablePtr rot;
	check_hresult(bindCtx->GetRunningObjectTable(&rot));
	IEnumMonikerPtr enumMoniker;
	check_hresult(rot->EnumRunning(&enumMoniker));
	IMonikerPtr nativityMoniker;
	check_hresult(CreateClassMoniker(CLSID_Nativity, &nativityMoniker));

	IMonikerPtr moniker;
	while (enumMoniker->Next(1, &moniker, nullptr) != S_FALSE) {
		if (nativityMoniker->IsEqual(moniker) != S_OK) {
			continue;
		}
		try {
			IUnknownPtr unkn;
			check_hresult(rot->GetObject(moniker, &unkn));
			INativityPtr nativity{ unkn };
			if (Callback(nativity)) {
				return true;
			}
		}
		catch (...) {

		}
	}
	return false;
}

bool NativityUtil::IsFileFiltered(const std::wstring& const file) {
	try {
		return Connect([file](INativityPtr nativity) -> bool
			{
				VARIANT_BOOL filtered;
				return SUCCEEDED(nativity->IsFiltered((LPWSTR)file.data(), &filtered)) && filtered != VARIANT_FALSE;
			});
	}
	catch (...) {
		return false;
	}
}

bool NativityUtil::OverlaysEnabled() {
	try {
		return Connect([](INativityPtr nativity) -> bool
			{
				VARIANT_BOOL enabled;
				return SUCCEEDED(nativity->OverlaysEnabled(&enabled)) && enabled != VARIANT_FALSE;
			});
	}
	catch (...) {
		return false;
	}
}

bool NativityUtil::ReceiveResponse(const std::wstring& const message, std::wstring& const response) {
	try {
		return Connect([message, &response](INativityPtr nativity) -> bool
			{
				bstr_handle responseHandle;
				if (FAILED(nativity->ReceiveMessage((LPWSTR)message.data(), responseHandle.put()))) {
					return false;
				}
				response.assign(responseHandle.get());
				return true;
			});
	}
	catch (...) {
		return false;
	}
}
