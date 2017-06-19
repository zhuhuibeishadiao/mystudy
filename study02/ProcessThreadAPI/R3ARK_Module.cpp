#include "stdafx.h"


/***************EnumProcessModules枚举模块*********************/
void ListProcessModules1(DWORD pid)
{
	HMODULE hMod[1024];
	DWORD cbNeeded;
	auto hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
	if (!hProcess)
		return;
	if (EnumProcessModules(hProcess, hMod, sizeof(hMod), &cbNeeded))
	{
		auto cMods = cbNeeded / sizeof(HMODULE);
		for (auto i = 0; i < cMods; i++)
		{
			WCHAR szName[MAX_PATH];
			if (GetModuleFileNameEx(hProcess, hMod[i], szName, MAX_PATH))
			{
				std::wcout << szName << std::endl;
			}
		}

	}
}
/***************************************************************/

/*************SnapShot枚举模块**************************/
void ListProcessModules2(DWORD pid)
{
	MODULEENTRY32 me32;
	me32.dwSize = sizeof(me32);
	auto hModSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pid);
	if (!Module32First(hModSnap, &me32))
		return;

	do 
	{
		std::wcout << me32.szModule << "\t";
		//std::wcout << me32.th32ModuleID << "\t";
		//std::wcout << me32.GlblcntUsage << "\t";
		//std::wcout << me32.ProccntUsage << "\t";
		//std::wcout << me32.modBaseAddr << "\t";
		//std::wcout << me32.modBaseSize << "\t";
		//std::wcout << me32.hModule << "\t";//BaseAddr与hModule是一回事
		std::wcout << me32.szExePath;
		std::cout << std::endl;
	} while (Module32Next(hModSnap, &me32));
}
/*******************************************************/
void TestARKModule()
{
	//ListProcessModules1(3328);
	ListProcessModules2(3328);
}