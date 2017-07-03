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