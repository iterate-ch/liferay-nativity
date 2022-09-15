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

bool Connect(com_ptr<INativity>& const nativity) {
	com_ptr<IObjectProvider> objectProvider;
	if (FAILED(CoGetClassObject(CLSID_ObjectProvider, CLSCTX_LOCAL_SERVER, 0, IID_PPV_ARGS(objectProvider.put())))) {
		return false;
	}
	return SUCCEEDED(objectProvider->QueryObject(CLSID_ObjectProvider, IID_PPV_ARGS(nativity.put())));
}

bool NativityUtil::IsFileFiltered(std::wstring file) {
	com_ptr<INativity> nativity;
	if (!Connect(nativity)) {
		return false;
	}

	VARIANT_BOOL filtered;
	if (FAILED(nativity->IsFiltered(file.data(), &filtered))) {
		return false;
	}
	return filtered;
}

bool NativityUtil::ReceiveResponse(std::wstring message, std::wstring& const response) {
	com_ptr<INativity> nativity;
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

//bool Connect(INativity& const nativity) {
//	com_ptr<IObjectProvider> provider;
//	if (FAILED(CoGetClassObject(CLSID_NativityEndpoint, CLSCTX_LOCAL_SERVER, NULL, IID_PPV_ARGS(provider.put())))) {
//		return false;
//	}
//
//	HRESULT result = provider->QueryObject(CLSID_NativityEndpoint, guid_of<INativity>(), (void**)&nativity);
//	if (FAILED(result)) {
//		return false;
//	}
//	return true;
//}
//
//bool NativityUtil::IsFileFiltered(const std::wstring const file) {
//	INativity nativity;
//	if (!Connect(nativity)) {
//		return false;
//	}
//
//	try {
//		return nativity.IsFiltered(file);
//	}
//	catch (...) {
//		return false;
//	}
//}
//
//bool NativityUtil::ReceiveResponse(const std::wstring const message, std::wstring& const response) {
//	INativity nativity;
//	if (!Connect(nativity)) {
//		return false;
//	}
//
//	try {
//		response = nativity.ReceiveMessage(message);
//		return true;
//	}
//	catch (...) {
//		return false;
//	}
//}
