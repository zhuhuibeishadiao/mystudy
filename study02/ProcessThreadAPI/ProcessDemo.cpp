#include "stdafx.h"

/***************创建进程实验*************************/
#define EXE_PATH L"C:\\Users\\killvxk\\Desktop\\procexp.exe"
void TestCreateProcess()
{
	STARTUPINFO si = {};
	PROCESS_INFORMATION pi = {};
	si.cb = sizeof(si);
	BOOL isSuc = CreateProcess(
		EXE_PATH,	//文件路径，若为空，第二个参数不能为空
		NULL,		//命令行，第一个空格之前是启动路径，空格之后是启动命令
		NULL,		//Security_Attributes，设为NULL则表示用默认安全属性
		NULL,		//线程安全属性，搞不懂，感觉不重要
		FALSE,		//子进程的句柄集成关系
		0,			//创建标志，一般不用。
		NULL,		//设为NULL使用父进程换进变量
		NULL,		//设为NULL使用父进程目录作为当前目录
		&si,		//STARTUPINFO（初始化位置等信息），可以设为NULL
		&pi			//创建成功后返回的信息
	);
	WaitForSingleObject(pi.hProcess, INFINITE);	//等待子进程结束
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
}
/***********************************************/

/*******************获得进程各项属性******************/
typedef struct _PROCESS_INFO
{
	DWORD dwPid;
	HANDLE hProcess;
	DWORD dwProVersion;
	DWORD dwPrioClass;
	DWORD dwHandleCount;
	DWORD_PTR dwAffinityMask;
	DWORD_PTR dwSystemAffinityMask;
	SIZE_T dwWorkingSetSizeMax;
	SIZE_T dwWorkingSetSizeMin;
	LPWSTR szCmdLine;
	STARTUPINFO sti;
}PROCESS_INFO, *LPPROCESS_INFO;
//成型的，以后可以自己添加
//
void GetProcessInfo()
{
	PROCESS_INFO pi;
	pi.hProcess = GetCurrentProcess();
	pi.dwPid = GetCurrentProcessId();
	pi.dwProVersion = GetProcessVersion(pi.dwPid);
	pi.dwPrioClass = GetPriorityClass(pi.hProcess);
	pi.dwHandleCount = GetProcessHandleCount(pi.hProcess, &pi.dwHandleCount);
	GetProcessAffinityMask(pi.hProcess, &pi.dwAffinityMask, &pi.dwSystemAffinityMask);
	GetProcessWorkingSetSize(pi.hProcess, &pi.dwWorkingSetSizeMax, &pi.dwWorkingSetSizeMin);
	pi.szCmdLine = GetCommandLine();
	GetStartupInfo(&pi.sti);
	
}
/******************************************************/

/*******************进程环境变量操作***********************/
void TestProcessEnvir()
{

	WCHAR value[MAX_PATH];
	ZeroMemory(value, MAX_PATH);
	GetEnvironmentVariable(L"USERNAME", value, MAX_PATH);
	printf("%ws\n", value);

	SetEnvironmentVariable(L"hama", L"hama");

	printf("========================================\n");
	LPTCH  pEvStart = GetEnvironmentStrings();
	LPWSTR pEv = (LPWSTR)pEvStart;
	do 
	{
		printf("%ws\n", pEv);
		while (*(pEv++));
	} while (*pEv);
	FreeEnvironmentStrings(pEvStart);

	

	
}
/**********************************************************/
void TestProcessDemo()
{
	//TestCreateProcess();
	//GetProcessInfo();
	TestProcessEnvir();
}