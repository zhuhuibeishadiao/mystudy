#include "stdafx.h"
#include <iostream>
using namespace std;

/*****************全局分配、局部分配********************/
//在32位上已经没有全局和举办函数概念，也不区分移动和固定概念
//所涉及的一些函数已经废弃不用了。所以只要记住以下用法就可以了。
void TestGlobalLocalMemDemo()
{
	auto lpMem = (LPVOID)GlobalAlloc(GPTR, 1000);
	auto  str = L"this is a string";
	memcpy(lpMem, str, wcslen(str) * 2 + 2);
	auto uFlags = GlobalFlags(lpMem);
	auto sizeMem = GlobalSize(lpMem);
	printf("内容为%ws, 地址为:0x%Ix, 大小：0x%I64x，内存属性:%u\n", (LPWSTR)lpMem, lpMem,sizeMem, uFlags);
	GlobalFree(lpMem);
	
}