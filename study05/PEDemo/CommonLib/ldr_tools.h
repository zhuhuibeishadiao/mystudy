#pragma once
#include "stdafx.h"
namespace usr::ldr::tools
{
	PVOID load_dll(std::wstring filename);
	void free_dll(HANDLE hMod);
	PVOID get_module_handle_process(DWORD64 ProcessId, 
		LPCTSTR lpszModuleName, 
		BOOL bGet32Module);
	ULONG_PTR get_proc_address(PVOID Image, 
		LPCSTR functionname);
	PVOID get_pro_address_process(
		DWORD64 ProcessId,
		LPCTSTR lpszModuleName,
		LPCSTR lpszApi,
		BOOL bGet32);
	PVOID get_pro_address_process(DWORD64 ProcessId,
		LPCTSTR lpszModuleName,
		LPCSTR lpszApi);

	PVOID get_section_by_name(PVOID ImageBase, std::string sec_name, SIZE_T & _size);

	PVOID get_peb_ldr_by_hmodule(PVOID hmodule);
};