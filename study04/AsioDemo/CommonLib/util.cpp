#include "stdafx.h"
#include <commdlg.h>
namespace usr::util
{
	void get_all_privilege()
	{
		using namespace ntdll;
		for (USHORT i = 0; i < 0x100; i++)
		{
			BOOLEAN Old;
			RtlAdjustPrivilege(i, TRUE, FALSE, &Old);
		}
	}
	void alloc_cmd_window()
	{
		AllocConsole();
		//SetConsoleTitle(_T("Êä³ö"));
		AttachConsole(GetCurrentProcessId());

		FILE* pFile = nullptr;
		freopen_s(&pFile, "CON", "r", stdin);
		freopen_s(&pFile, "CON", "w", stdout);
		freopen_s(&pFile, "CON", "w", stderr);
	}
	void hex2string(_tstring &string_, LPVOID data_, std::size_t size_)
	{
		string_ = _T("");
		const TCHAR IntegerToChar[] = _T("0123456789abcdef"); /*0-16*/
		auto buffer_ = reinterpret_cast<PUCHAR>(data_);
		for (SIZE_T i = 0; i < size_; i++)
		{
			TCHAR szString[3] = {};
			szString[0] = IntegerToChar[buffer_[i] >> 4];
			szString[1] = IntegerToChar[buffer_[i] & 0xf];
			string_ += szString;
		}
		return;
	}
	void string2hex(_tstring string_, std::vector<BYTE> &data_)
	{
		const int CharToInteger[256] =
		{
			-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, /* 0 - 15 */
			-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, /* 16 - 31 */
			36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, /* ' ' - '/' */
			0, 1, 2, 3, 4, 5, 6, 7, 8, 9, /* '0' - '9' */
			52, 53, 54, 55, 56, 57, 58, /* ':' - '@' */
			10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, /* 'A' - 'Z' */
			59, 60, 61, 62, 63, 64, /* '[' - '`' */
			10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, /* 'a' - 'z' */
			65, 66, 67, 68, -1, /* '{' - 127 */
			-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, /* 128 - 143 */
			-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, /* 144 - 159 */
			-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, /* 160 - 175 */
			-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, /* 176 - 191 */
			-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, /* 192 - 207 */
			-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, /* 208 - 223 */
			-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, /* 224 - 239 */
			-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 /* 240 - 255 */
		};
		auto length = string_.length() / 2;
		data_.resize(length);
		for (SIZE_T i = 0; i < length; i++)
		{
			data_[i] =
				(UCHAR)(CharToInteger[(UCHAR)string_[i * 2]] << 4) +
				(UCHAR)CharToInteger[(UCHAR)string_[i * 2 + 1]];
		}
		return;
	}
	DWORD GetOpenName(HINSTANCE hInstance, TCHAR* outbuf, const TCHAR* filter, const TCHAR* title)
	{
		OPENFILENAME ofn;
		memset(&ofn, 0, sizeof(OPENFILENAME));

		TCHAR buf[MAX_PATH + 2] = {};
		GetModuleFileName(hInstance, buf, MAX_PATH);

		TCHAR* tmp = StrRChr(buf, NULL, L'\\');
		if (tmp != 0)
		{
			*tmp = 0;
			ofn.lpstrInitialDir = buf;
		}

		ofn.hInstance = hInstance;
		ofn.hwndOwner = NULL;
		ofn.lStructSize = sizeof(OPENFILENAME);
		ofn.lpstrFilter = filter;
		ofn.nFilterIndex = 1;
		ofn.lpstrFile = outbuf;
		ofn.lpstrFile[0] = 0;
		ofn.lpstrFile[1] = 0;
		ofn.nMaxFile = MAX_PATH;
		ofn.lpstrTitle = title;
		ofn.Flags = OFN_EXPLORER | OFN_DONTADDTORECENT | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_LONGNAMES | OFN_NONETWORKBUTTON | OFN_PATHMUSTEXIST;

		return GetOpenFileName(&ofn);
	}
	DWORD GetSaveName(HINSTANCE hInstance, TCHAR* outbuf, const TCHAR* filter, const TCHAR* title)
	{
		OPENFILENAME ofn;
		memset(&ofn, 0, sizeof(OPENFILENAME));

		TCHAR buf[MAX_PATH + 2] = {};
		GetModuleFileName(hInstance, buf, MAX_PATH);

		TCHAR* tmp = StrRChr(buf, NULL, L'\\');
		if (tmp != 0)
		{
			*tmp = 0;
			ofn.lpstrInitialDir = buf;
		}

		ofn.hInstance = hInstance;
		ofn.hwndOwner = NULL;
		ofn.lStructSize = sizeof(OPENFILENAME);
		ofn.lpstrFilter = filter;
		ofn.nFilterIndex = 1;
		ofn.lpstrFile = outbuf;
		ofn.lpstrFile[0] = 0;
		ofn.lpstrFile[1] = 0;
		ofn.nMaxFile = MAX_PATH;
		ofn.lpstrTitle = title;
		ofn.Flags = OFN_EXPLORER | OFN_DONTADDTORECENT |OFN_LONGNAMES | OFN_NONETWORKBUTTON ;

		return GetSaveFileName(&ofn);
	}
}