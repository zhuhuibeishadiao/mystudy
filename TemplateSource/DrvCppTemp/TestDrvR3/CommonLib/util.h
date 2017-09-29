#pragma once
//#include "stdafx.h"

namespace usr::util
{
	void get_all_privilege();
	void alloc_cmd_window();
	void hex2string(_tstring &string_, LPVOID data_, std::size_t size_);
	void string2hex(_tstring string_, std::vector<BYTE> &data_);
	//DWORD32 rot13
	DWORD32 crc32(LPVOID data_, std::size_t size_);
	//bool crack_crc32_for_dwordptr
	void loadfile2vec(_tstring file_name, std::vector<BYTE> &data_);
	DWORD GetSaveName(HINSTANCE hInstance, TCHAR* outbuf, const TCHAR* filter, const TCHAR* title);
	DWORD GetOpenName(HINSTANCE hInstance, TCHAR* outbuf, const TCHAR* filter, const TCHAR* title);
}