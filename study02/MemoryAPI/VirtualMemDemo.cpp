#include "stdafx.h"
#include <iostream>


/*****************虚拟内存的分配和释放****************************/
void VirtualAllocAndFree()
{
	SIZE_T sizeVirtual = 0x3000;
	LPVOID lpRound = reinterpret_cast<LPVOID>(0x10000001);
	MEMORY_BASIC_INFORMATION mbi;
	auto lpAddress = VirtualAlloc(lpRound, sizeVirtual, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	CopyMemory(lpAddress, "hello", strlen("hello"));
	std::cout << "分配地址为：" << lpAddress << "  ";
	std::cout << "内容为：" << (LPSTR)lpAddress << std::endl;;

	//备注：因为起始为10000001+size横跨2个页面,所以查出来的RegionSize页面多一个
	VirtualQuery(lpAddress, &mbi, sizeof(mbi));//查看mbi的state值 0x1000 MEM_COMMIT

	VirtualFree(lpRound, sizeVirtual, MEM_DECOMMIT);
	VirtualQuery(lpAddress, &mbi, sizeof(mbi));//查看mbi的state值 0x2000 MEM_DECOMMIT

	VirtualFree(lpAddress, 0, MEM_RELEASE);
	VirtualQuery(lpAddress, &mbi, sizeof(mbi));//查看mbi的state值 0x00010000 MEM_FREE
}
/*****************************************************************/

/********************修改内存属性******************************/
void VirtualProtectDemo()
{
	MEMORY_BASIC_INFORMATION mbi;
	DWORD oldPro;
	auto lpMem = VirtualAlloc(NULL, 0x1001, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	VirtualQuery(lpMem, &mbi, sizeof(mbi));

	VirtualProtect(lpMem, 0x2000, PAGE_READONLY, &oldPro);
	VirtualQuery(lpMem, &mbi, sizeof(mbi));
}
/*******************************************************/
void TestVirtualMemDemo()
{
	//VirtualAllocAndFree();
	VirtualProtectDemo();
}