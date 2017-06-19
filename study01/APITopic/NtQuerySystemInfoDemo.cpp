#include "NtQuerySystemInfoDemo.h"
#include "stdafx.h"
using namespace std;

void NTQSIDemo1()
{
	//SystemBasicInformation查CPU核数
	SYSTEM_BASIC_INFORMATION bi = {};
	auto len = sizeof(SYSTEM_BASIC_INFORMATION);
	NtQuerySystemInformation(NTDLL::SystemBasicInformation, &bi, len, 0);
	cout << "CPU核数：" << static_cast<int>(bi.NumberOfProcessors) << endl;
}

/************************************************************************
* 函数功能：找出进程名为lpszName的所有进程pid，并以向量方式返回
* 参数一：要查找的进程名         参数二：存放返回的vector
************************************************************************/
void find_process(LPCWSTR lpszName, std::vector<DWORD>& pid_list)
{
	//SystemProcessInformation枚举进程
	//SystemExtendedProcessInformation也可以作为第一个参数
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
		if (!buffer)return;
		status = NtQuerySystemInformation(NTDLL::SystemExtendedProcessInformation, buffer, len, 0);

	} while (status != STATUS_SUCCESS);

	pspi = (NTDLL::PSYSTEM_PROCESSES_INFORMATION)buffer;
	WCHAR szName[MAX_PATH] = { 0 };

	while (pspi->NextEntryDelta)
	{
		//std::wcout << ((pspi->ImageName.Buffer == NULL) ? L"System Idle Process": pspi->ImageName.Buffer) << std::endl;
		RtlCopyMemory(szName, pspi->ImageName.Buffer, pspi->ImageName.MaximumLength);
		if (_wcsicmp(szName, lpszName) == 0)//找到
		{
			auto pid = reinterpret_cast<DWORD>(pspi->UniqueProcessId);
			if (pid != GetCurrentProcessId())
			{
				pid_list.push_back(pid);
			}
		}
		pspi = (NTDLL::PSYSTEM_PROCESSES_INFORMATION)(reinterpret_cast<ULONG>(pspi) + pspi->NextEntryDelta);
	}
	LocalFree(buffer);
}

/************************************************************************
* 函数功能：SystemExtendedProcessInformation返回指定进程的主线程
* 参数一：进程ID
* 返回值：线程id,返回0失败
************************************************************************/
DWORD GetMainThread(DWORD pid)
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
		if (pspi->UniqueProcessId == (HANDLE)pid)
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
