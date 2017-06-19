#include "stdafx.h"

//inject_remote_thread 远程线程注入
bool inject_remote_thread(DWORD dwPid, LPCWSTR lpszDllPath)
{
	HANDLE h_process = NULL;
	HANDLE hThread = NULL;
	SIZE_T dwRet = 0;
	WCHAR szPath[MAX_PATH] = { 0 };

	h_process = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwPid);
	if (!h_process)
		return false;
	wcscpy_s(szPath,MAX_PATH, lpszDllPath);
	auto lpstr = VirtualAllocEx(h_process, NULL, MAX_PATH, MEM_COMMIT, PAGE_READWRITE);
	if (lpstr == NULL)
		return false;
	WriteProcessMemory(h_process, lpstr, szPath, MAX_PATH, &dwRet);
	auto h_kernel32 = GetModuleHandle(_T("kernel32.dll"));
	auto pfn_caller = GetProcAddress(h_kernel32, "LoadLibraryW");
	hThread = CreateRemoteThread(h_process, NULL, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(pfn_caller), lpstr, 0, NULL);
	if (!hThread)
		return false;
	
	CloseHandle(hThread);
	CloseHandle(h_process);
	return true;

}