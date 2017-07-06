#pragma once
//#include "stdafx.h"

namespace usr::util
{
	void get_all_privilege();
	void alloc_cmd_window();
	DWORD GetSaveName(HINSTANCE hInstance, TCHAR* outbuf, const TCHAR* filter, const TCHAR* title);
	DWORD GetOpenName(HINSTANCE hInstance, TCHAR* outbuf, const TCHAR* filter, const TCHAR* title);
}