#include <objbase.h>
#include <json/json.h>
#include <StringUtil.h>
#include <NativityUtil.h>
#include <winrt/base.h>

using namespace std;
using namespace Nativity::Util;

int main()
{
	CoInitializeEx(0, COINIT_APARTMENTTHREADED);
	NativityUtil nativity;
	if (!NativityUtil::Find(L"", nativity)) {
		printf_s("Failure finding Nativity.");
		return 1;
	}

	auto enabled{ nativity->OverlaysEnabled() };
	printf_s("%d", enabled);

	Json::Value jsonRoot;
	jsonRoot["test"] = "L";
	Json::FastWriter writer;
	wstring message;
	auto json{ writer.write(jsonRoot) };
	message = StringUtil::toWstring(json);
	wstring response;
	nativity->ReceiveResponse(message, response);
	return 0;
}
