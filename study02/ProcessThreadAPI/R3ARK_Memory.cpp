#include "stdafx.h"

/******************进程内存使用情况***********************/
void ListMemoryInfo(DWORD pid)
{
	PROCESS_MEMORY_COUNTERS pmc;
	auto hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
	if (hProcess == NULL)
		return;
	if (GetProcessMemoryInfo(hProcess, &pmc, sizeof(pmc)))
	{
		std::cout << "PageFaultCount:" << pmc.PageFaultCount << std::endl;;
		std::cout << "PeakWorkingSetSize:" << pmc.PeakWorkingSetSize << std::endl;
		std::cout << "WorkingSetSize:" << pmc.WorkingSetSize << std::endl;
		std::cout << "QuotaPeakPagedPoolUsage:" << pmc.QuotaPeakPagedPoolUsage << std::endl;
		std::cout << "QuotaPagedPoolUsage:" << pmc.QuotaPagedPoolUsage << std::endl;
		std::cout << "QuotaPeakNonPagedPoolUsage:" << pmc.QuotaPeakNonPagedPoolUsage << std::endl;
		std::cout << "QuotaNonPagedPoolUsage:" << pmc.QuotaNonPagedPoolUsage << std::endl;
		std::cout << "PagefileUsage:" << pmc.PagefileUsage << std::endl;
		std::cout << "PeakPagefileUsage:" << pmc.PeakPagefileUsage << std::endl;
	}
}
/*********************************************************/

/******************进程堆使用情况*************************/
void ListHeapInfo(DWORD pid)
{
	HEAPLIST32 hl32;
	hl32.dwSize = sizeof(hl32);
	HEAPENTRY32 he32;
	he32.dwSize = sizeof(he32);

	auto hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPHEAPLIST, pid);
	if (!Heap32ListFirst(hSnap, &hl32))
		return;
	do 
	{
		std::cout << "==================================" << std::endl;
		std::cout << hl32.th32HeapID << "\t" << hl32.dwFlags << std::endl;
		if (!Heap32First(&he32, pid, hl32.th32HeapID))
		{
			CloseHandle(hSnap);
			return;
		}
		do 
		{
			std::cout << "Heap Address = " << he32.dwAddress << "\t";
			std::cout << "Heap Size = " << he32.dwBlockSize << "\t";
			std::cout << "Heap Flag = " << he32.dwFlags << "\t";
			std::cout << "Heap Handle = " << he32.hHandle << "\t";
			std::cout << std::endl;
		} while (Heap32Next(&he32));
		
	} while (Heap32ListNext(hSnap, &hl32));
	CloseHandle(hSnap);
}
/*********************************************************/
void TestMemoryDemo()
{
	//ListMemoryInfo(3328);
	ListHeapInfo(5408);
}