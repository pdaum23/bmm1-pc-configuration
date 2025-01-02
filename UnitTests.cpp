
#include "stdafx.h"
#include "CppUnitTest.h"
#include <afxwin.h>
#include "udl1config.h"
#include "udl1configDlg.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

template <typename Q> static std::wstring ToString(const Q& q)
{
	RETURN_WIDE_STRING(t);
}

namespace UnitTests
{
	TEST_CLASS(UnitTests)
	{

	public:
		
		TEST_METHOD(TestMethodNotesRemoveSDLInfo)
		{
			CString const result = "Blah blah";
			CString const empty = "";
			CString const starting1 = "Blah Blah\rS1C9"; // expect result
			CString const starting2 = "\rS1C129"; // expect empty string
			CString starting3 = "Blah Blah"; // expect result
			CString starting4 = ""; // expect empty string

			Assert::AreEqual(Cudl1configDlg::NotesRemoveSDLInfo(starting1), result, "test1");
			Assert::AreEqual(Cudl1configDlg::NotesRemoveSDLInfo(starting2), empty, "test2");
			Assert::AreEqual(Cudl1configDlg::NotesRemoveSDLInfo(starting3), result, "test3");
			Assert::AreEqual(Cudl1configDlg::NotesRemoveSDLInfo(starting4), empty, "test4");
		}
	};
}
