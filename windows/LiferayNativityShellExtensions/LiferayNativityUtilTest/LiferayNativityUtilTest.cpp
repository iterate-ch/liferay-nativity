#include "pch.h"
#include <Unknwn.h>
#include "CppUnitTest.h"
#include <NativityUtil.h>

using namespace std;
using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace LiferayNativityUtilTest
{
	TEST_CLASS(LiferayNativityUtilTest)
	{
	public:
		
		TEST_METHOD(TestCoAuth)
		{
		}

		TEST_METHOD(TestComFiltered)
		{
		}

		TEST_METHOD(TestComMessage)
		{
			CoInitializeEx(0, COINIT_MULTITHREADED);
			wstring response;
			Assert::IsTrue(NativityUtil::ReceiveResponse(L"TEST", response));
		}
	};
}
