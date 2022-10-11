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

bool Connect(INativityPtr& nativity) {
	auto objectProvider{ try_create_instance<IObjectProvider>(CLSID_ObjectProvider, CLSCTX_ALL) };
	if (!objectProvider) {
		return false;
	}
	return SUCCEEDED(objectProvider->QueryObject(CLSID_Nativity, IID_PPV_ARGS(&nativity)));
}

bool NativityUtil::IsFileFiltered(std::wstring file) {
	INativityPtr nativity;
	if (!Connect(nativity)) {
		return false;
	}

	VARIANT_BOOL filtered;
	try {
		check_hresult(nativity->IsFiltered(file.data(), &filtered));
	}
	catch (...) {
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
	try {
		check_hresult(nativity->OverlaysEnabled(&enabled));
	}
	catch (...) {
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
	try {
		check_hresult(nativity->ReceiveMessage(message.data(), responseHandle.put()));
	}
	catch (...) {
		return false;
	}
	response.assign(responseHandle.get());
	return true;
}
