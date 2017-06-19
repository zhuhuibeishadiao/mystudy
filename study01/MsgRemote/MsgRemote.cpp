// MsgRemote.cpp : 远程线程消息传递
//

#include "stdafx.h"
#include "common.h"
#include <iostream>

void run_server1();
void GetAllPrivilege();
void find_process(LPCWSTR lpszName, std::vector<DWORD> &pid_list);

DWORD WINAPI RemoteMsgProc(LPVOID Context)
{
	int pid = reinterpret_cast<int>(Context);
	printf("hello world, %d", pid);
	return 0;
}

int main()
{
	do 
	{
		auto h_event = CreateEvent(NULL, FALSE, FALSE, _T("Father"));
		if (!h_event)break;
		if (GetLastError() == ERROR_ALREADY_EXISTS)
		{
			std::cout << "wait msg" << std::endl;
			while (true)
				Sleep(1);
		}
		else
		{
			run_server1();
			break;
		}
	} while (true);
	
    return 0;
}

void run_server1()
{

	std::cout << "Wait Client" << std::endl;
	std::vector<DWORD> pid_insert;
	int pid = 0;//
	while (true)
	{
		std::vector<DWORD> pidList;
		find_process(_T("MsgRemote.exe"), pidList);


		if (!pidList.empty())
		{
			for (auto pid_item : pidList)
			{
				
				//在pid_insert里找，找不到则插入，找到了说明不用给新创建的进程发消息。
				auto itPid = find(pid_insert.begin(), pid_insert.end(), pid_item);
				if (itPid == pid_insert.end())//找不到指向最后
				{
					auto h_Process = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid_item);
					if (h_Process == NULL) return;
					HMODULE hModule = 0;
					auto delta = reinterpret_cast<ULONG_PTR>(RemoteMsgProc) - reinterpret_cast<ULONG_PTR>(GetModuleHandle(NULL));
					EnumProcessModules(h_Process, &hModule, sizeof(hModule), NULL);
					if (hModule)
					{
						MODULEINFO minfo = { 0 };
						GetModuleInformation(h_Process, hModule, &minfo, sizeof(minfo));
						delta += reinterpret_cast<ULONG_PTR>(minfo.lpBaseOfDll);
					}
					else
					{
						delta += reinterpret_cast<ULONG_PTR>(GetModuleHandle(NULL));
					}
					CreateRemoteThread(h_Process, NULL, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(delta), reinterpret_cast<LPVOID>(pid_item), 0, NULL);
					pid_insert.push_back(pid_item);
					CloseHandle(h_Process);
				}
			}
		}
		Sleep(500);
	}
}

//提升权限(提成整个进程权限)
void GetAllPrivilege()
{
	for (ULONG i = 0; i < 0x100; i++)
	{
		BOOLEAN old;
		RtlAdjustPrivilege(i, TRUE, FALSE, &old);
	}
}

void find_process(LPCWSTR lpszName, std::vector<DWORD>& pid_list)
{
	ULONG len = 0;
	NTSTATUS status;
	PVOID buffer;
	NTDLL::PSYSTEM_PROCESSES_INFORMATION pspi;
	NtQuerySystemInformation(NTDLL::SystemExtendedProcessInformation, NULL, 0, &len);
	
	do 
	{
		len += 1000;
		buffer = LocalAlloc(LMEM_FIXED, len);
		if (!buffer)return;
		status = NtQuerySystemInformation(NTDLL::SystemExtendedProcessInformation, buffer, len, 0);

	} while (status != STATUS_SUCCESS);

	pspi = (NTDLL::PSYSTEM_PROCESSES_INFORMATION)buffer;
	WCHAR szName[MAX_PATH] = { 0 };
	


	while (pspi->NextEntryDelta)
	{
		//std::wcout << ((pspi->ImageName.Buffer == NULL) ? L"aaa": pspi->ImageName.Buffer) << std::endl;
		RtlCopyMemory(szName, pspi->ImageName.Buffer, pspi->ImageName.MaximumLength);
		if (wcsicmp(szName, lpszName) == 0)//找到
		{
			auto pid = reinterpret_cast<DWORD>(pspi->UniqueProcessId);
			if (pid != GetCurrentProcessId())
			{
				pid_list.push_back(pid);
			}
		}
		pspi = (NTDLL::PSYSTEM_PROCESSES_INFORMATION)(reinterpret_cast<ULONG>(pspi) + pspi->NextEntryDelta);
	}
}




