#include "stdafx.h"

unsigned long GetMainThreadId(unsigned long ProcessId)
{
	//如果缓冲大小不够，则会影响返回结果
	ULONG len = 0;
	NTSTATUS status;
	PVOID buffer;
	NTDLL::PSYSTEM_PROCESSES_INFORMATION pspi;
	//先大概求出一共需要多少字节
	NtQuerySystemInformation(NTDLL::SystemExtendedProcessInformation, NULL, 0, &len);

	do
	{
		//然后长度加上一个保险值，自己预测大概300
		len += 300;
		buffer = LocalAlloc(LMEM_FIXED, len);
		if (!buffer) return NULL;
		status = NtQuerySystemInformation(NTDLL::SystemExtendedProcessInformation, buffer, len, 0);

	} while (status != STATUS_SUCCESS);

	pspi = (NTDLL::PSYSTEM_PROCESSES_INFORMATION)buffer;

	while (pspi->NextEntryDelta)
	{
		if (pspi->UniqueProcessId == (HANDLE)ProcessId)
		{
			DWORD threaId = reinterpret_cast<DWORD>(pspi->Threads[0].ClientId.UniqueThread);
			LocalFree(buffer);
			return threaId;
		}
		pspi = (NTDLL::PSYSTEM_PROCESSES_INFORMATION)(reinterpret_cast<ULONG>(pspi) + pspi->NextEntryDelta);
	}
	LocalFree(buffer);
	return NULL;
}
//////////////////////////////////////////////////////////////////////////
#pragma optimize( "", off ) 
__declspec(naked) void stub1()
{
	__asm
	{
		pushad
		pushfd
		call start

		start :
		pop ecx
		sub ecx, 7

		lea eax, [ecx + 32]
		push eax
		call dword ptr[ecx - 4]

		popfd
		popad
		ret
	}
}

DWORD WINAPI stub_end1()
{
	return 0;
}
#pragma optimize("", on )

PVOID WriteStubEx1(HANDLE hProcess, LPCWSTR lpszDllFilePath)
{
	ULONG_PTR stublen;
	PVOID LoadLibAddr, mem;

	stublen = (ULONG_PTR)stub_end1 - (ULONG_PTR)stub1;
	LoadLibAddr = GetProcAddress(GetModuleHandle(_T("kernel32.dll")), "LoadLibraryW");
	mem = VirtualAllocEx(hProcess, NULL, 4096, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);

	//printf("Memory allocated at %p\nAbout to write stub code...\n", mem);
	WriteProcessMemory(hProcess, mem, &LoadLibAddr, sizeof(PVOID), NULL);
	WriteProcessMemory(hProcess, (LPVOID)((LPBYTE)mem + 4), stub1, stublen, NULL);
	WriteProcessMemory(hProcess, (LPVOID)((LPBYTE)mem + 4 + stublen), lpszDllFilePath, MAX_PATH * sizeof(WCHAR), NULL);
	return (PVOID)((LPBYTE)mem + 4);
}

//修改EIP注入inject_context_dll
BOOL inject_context_dll(DWORD dwProcessId, LPCWSTR lpszDllFilePath)
{
	auto MainThreadId = GetMainThreadId(dwProcessId);
	auto ret = FALSE;
	auto h_process = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwProcessId);
	if (h_process && h_process != INVALID_HANDLE_VALUE)
	{
		WCHAR szName[MAX_PATH] = { 0 };
		wcscpy_s(szName, MAX_PATH, lpszDllFilePath);
		auto h_Thread = OpenThread(THREAD_ALL_ACCESS, FALSE, MainThreadId);
		if (h_Thread && h_Thread != INVALID_HANDLE_VALUE)
		{
			auto dwRet = SuspendThread(h_Thread);
			if (dwRet != (DWORD)-1)
			{
				auto mem = WriteStubEx1(h_process, szName);
				CONTEXT ctx;
				ctx.ContextFlags = CONTEXT_FULL;
				if (GetThreadContext(h_Thread, &ctx))
				{
					ctx.Esp -= 4;
					WriteProcessMemory(h_process, reinterpret_cast<PVOID>(ctx.Esp), &ctx.Eip, sizeof(PVOID), NULL);
					ctx.Eip = reinterpret_cast<DWORD>(mem);
					if (SetThreadContext(h_Thread, &ctx))
						ret = TRUE;
				}
				ResumeThread(h_Thread);
			}
			CloseHandle(h_Thread);
		}
		CloseHandle(h_process);
	}
	return ret;
}
