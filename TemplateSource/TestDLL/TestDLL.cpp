// TestDLL.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"

/**************************************
  直接使用__declspec(dllexport)导出测试
  
**************************************/
//#include "../TempDLL/Header.h"
//#pragma comment(lib, "../x64/Debug/TempDLL.lib")
//void TestTempDLL()
//{
//	abcd();
//	abcd(11);
//}

/**************************************
 使用def文件导出
**************************************/
#include "../TempDLLDef/Header.h"
#pragma comment(lib, "../x64/Debug/TempDLLDef.lib")
void TestTempDLLDef()
{
	abc();
	abcd();
}


int main()
{
	//TestTempDLL();
	TestTempDLLDef();
    return 0;
}

