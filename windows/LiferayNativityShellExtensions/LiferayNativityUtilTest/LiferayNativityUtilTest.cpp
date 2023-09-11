#include "CppUnitTest.h"
#include <NativityUtil.h>
#include <Windows.h>

using namespace std;
using namespace Nativity::Util;
using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace LiferayNativityUtilTest
{
	TEST_CLASS(LiferayNativityUtilTest)
	{
	public:
		
		TEST_METHOD(TestCoAuth)
		{
		}

		TEST_METHOD(TestComOverlaysEnabled)
		{
			CoInitializeEx(0, COINIT_MULTITHREADED);
			NativityUtil ptr;
			if (!NativityUtil::Find(L"", ptr)) {
				return;
			}

			ptr->OverlaysEnabled();
		}

		TEST_METHOD(TestComFiltered)
		{
			CoInitializeEx(0, COINIT_MULTITHREADED);
			NativityUtil ptr;
			if (!NativityUtil::Find(L"", ptr)) {
				return;
			}
		}
	};
}
