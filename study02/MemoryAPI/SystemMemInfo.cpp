#include "stdafx.h"
#include <iostream>
using namespace std;
void ShowSystemMemInfo()
{
	MEMORYSTATUSEX memstat;
	memstat.dwLength = sizeof(memstat);
	GlobalMemoryStatusEx(&memstat);
	cout << "总物理内存:" << memstat.ullTotalPhys;
	cout << " 可用物理内存:" << memstat.ullAvailPhys;
	cout << " 使用百分比:" << memstat.dwMemoryLoad << endl;;

	cout << "总分页:" << memstat.ullTotalPageFile;
	cout << " 可用分页:" << memstat.ullAvailPageFile << endl;;

	cout << "总虚拟内存" << memstat.ullTotalVirtual;
	cout << " 可用虚拟内存" << memstat.ullAvailVirtual << endl;
}