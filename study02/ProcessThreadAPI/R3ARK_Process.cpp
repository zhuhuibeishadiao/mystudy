#include "stdafx.h"


/*************EnumProcess枚举进程************************/
//摘自MSDN
void PrintNameAndModules(DWORD pid)
{
	WCHAR szName[MAX_PATH] = {};
	HANDLE h_process = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);

	if (NULL != h_process)
	{
		HMODULE h_mod;//返回的是一个数组，但我们这里只需要知道名字所以定义一个就行
		DWORD cbNeed;
		if (EnumProcessModules(h_process, &h_mod, sizeof(h_mod), &cbNeed))
		{
			GetModuleBaseName(h_process, h_mod, szName, MAX_PATH);
		}
	}
	std::wcout << pid << ":" << szName << std::endl;
	CloseHandle(h_process);
}
void EnumProDemo1()
{
	// Get the list of process identifiers.
	DWORD aProcesses[1024], cbNeeded, cProcesses;
	if (!EnumProcesses(aProcesses, sizeof(aProcesses), &cbNeeded))
	{
		return;
	}
	// Calculate how many process identifiers were returned.
	cProcesses = cbNeeded / sizeof(DWORD);

	for (auto i = 0; i < cProcesses; i++)
	{
		if (aProcesses[i] != 0)
		{
			PrintNameAndModules(aProcesses[i]);
		}
	}
	std::cout << "一共" << cProcesses << "个进程" << std::endl;
	getchar();
}
/*******************************************************/

/*************SnapShot枚举进程**************************/
void EnumProDemo2()
{
	PROCESSENTRY32 pe32;
	auto hProcSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);//0代码枚举所有进程
	if (hProcSnap == INVALID_HANDLE_VALUE)
		return;
	pe32.dwSize = sizeof(pe32);

	if (!Process32First(hProcSnap, &pe32))
		return;
	do 
	{
		std::cout << "=====================================" << std::endl;
		std::wcout << pe32.szExeFile << ":" << pe32.th32ProcessID << std::endl;
		std::cout << "线程总数:" << pe32.cntThreads << std::endl;
		std::cout << "父进程:" << pe32.th32ParentProcessID <<std::endl;
		std::cout << "优先级:" << pe32.pcPriClassBase << std::endl;
	} while (Process32Next(hProcSnap, &pe32));
	CloseHandle(hProcSnap);
	getchar();
}
/*******************************************************/

/*******************通过进程名获得PID*************************/
BOOL GetProcessIdByName(LPCWSTR szProName, LPDWORD lpPID)
{
	PROCESSENTRY32 pe32;
	pe32.dwSize = sizeof(pe32);
	auto hProcSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hProcSnap == INVALID_HANDLE_VALUE)
		return FALSE;
	if (!Process32First(hProcSnap, &pe32))
		return FALSE;

	do 
	{
		if (wcscmp(szProName, pe32.szExeFile) == 0)
		{
			*lpPID = pe32.th32ProcessID;
			CloseHandle(hProcSnap);
			return TRUE;
		}
	} while (Process32Next(hProcSnap, &pe32));
	CloseHandle(hProcSnap);
	return FALSE;
}
/*******************************************************/
void TestARKProcess()
{
	//EnumProDemo1();
	//EnumProDemo2();

	DWORD pid = 0;
	GetProcessIdByName(L"360Tray.exe", &pid);
}