#include "NativityUtil.h"
#include <ShObjIdl.h>
#include <winrt/base.h>
#include "UtilConstants.h"

#import <INativity.tlb> no_namespace

class DECLSPEC_UUID("8FE0A7E5-25A4-45A3-9694-37ED25D333C7")
	ObjectProvider;
constexpr CLSID CLSID_ObjectProvider = __uuidof(ObjectProvider);
class DECLSPEC_UUID("3B512D20-7C95-44ED-A0A9-267C0AC9A428")
	Nativity;
constexpr CLSID CLSID_Nativity = __uuidof(Nativity);

using namespace winrt;
using namespace impl;

_COM_SMARTPTR_TYPEDEF(IObjectProvider, IID_IObjectProvider);

bool Connect(INativityPtr& nativity) {
	IObjectProviderPtr objectProvider { CLSID_ObjectProvider };
	return SUCCEEDED(objectProvider->QueryObject(CLSID_Nativity, IID_PPV_ARGS(&nativity)));
}

bool NativityUtil::IsFileFiltered(std::wstring file) {
	INativityPtr nativity;
	if (!Connect(nativity)) {
		return false;
	}

	VARIANT_BOOL filtered;
	if (FAILED(nativity->IsFiltered(file.data(), &filtered))) {
		return false;
	}
	return filtered;
}

bool NativityUtil::OverlaysEnabled() {
	INativityPtr nativity;
	if (!Connect(nativity)) {
		return false;
	}

	VARIANT_BOOL enabled;
	if (FAILED(nativity->OverlaysEnabled(&enabled))) {
		return false;
	}
	return enabled;
}

bool NativityUtil::ReceiveResponse(std::wstring message, std::wstring& const response) {
	INativityPtr nativity;
	if (!Connect(nativity)) {
		return false;
	}

	bstr_handle responseHandle;
	if (FAILED(nativity->ReceiveMessage(message.data(), responseHandle.put()))) {
		return false;
	}
	response.assign(responseHandle.get());
	return true;
}
