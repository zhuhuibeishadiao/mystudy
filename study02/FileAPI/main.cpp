#include "stdafx.h"




extern void TestFileFuncDemo();
int main()
{
	TestFileFuncDemo();
	return 0;
}





/************************************************************************/
/*       C++控制台乱码解决                                                               
 * 1. C函数设置全局locale
 * setlocale(LC_ALL, "");

 * 2. C++ 设置全局locale
 * std::locale::global(std::locale(""));

 * 3. 单独为 wcout 设置一个 locale
 * std::locale loc("");
 * std::wcout.imbue(loc);
/************************************************************************/