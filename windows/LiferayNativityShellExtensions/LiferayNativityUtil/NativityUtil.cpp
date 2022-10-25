#include "NativityUtil.h"
#include <comdef.h>
#include <functional>
#include "UtilConstants.h"

#include "INativity.h"

class DECLSPEC_UUID("3B512D20-7C95-44ED-A0A9-267C0AC9A428")
	Nativity;
constexpr CLSID CLSID_Nativity = __uuidof(Nativity);

_COM_SMARTPTR_TYPEDEF(INativity, IID_INativity);

bool Connect(std::function<bool(INativityPtr)> Callback) {
	IBindCtxPtr bindCtx;
	throw_if_fail(CreateBindCtx(0, &bindCtx));
	IRunningObjectTablePtr rot;
	throw_if_fail(bindCtx->GetRunningObjectTable(&rot));
	IEnumMonikerPtr enumMoniker;
	throw_if_fail(rot->EnumRunning(&enumMoniker));
	IMonikerPtr nativityMoniker;
	throw_if_fail(CreateClassMoniker(CLSID_Nativity, &nativityMoniker));

	IMonikerPtr moniker;
	while (enumMoniker->Next(1, &moniker, nullptr) != S_FALSE) {
		if (nativityMoniker->IsEqual(moniker) != S_OK) {
			continue;
		}
		IUnknownPtr unkn;
		if (FAILED(rot->GetObject(moniker, &unkn))) {
			continue;
		}

		try {
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
		return Connect([&file](INativityPtr nativity) -> bool
			{
				VARIANT_BOOL filtered;
				return SUCCEEDED(nativity->IsFiltered(file.c_str(), &filtered)) && filtered != VARIANT_FALSE;
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
		return Connect([&message, &response](INativityPtr nativity) -> bool
			{
				_bstr_t marshal;
				if (FAILED(nativity->ReceiveMessage(message.c_str(), marshal.GetAddress()))) {
					return false;
				}
				response.assign(marshal);
				return true;
			});
	}
	catch (...) {
		return false;
	}
}
