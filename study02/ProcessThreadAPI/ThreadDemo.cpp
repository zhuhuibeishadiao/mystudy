#include "stdafx.h"


/************************创建线程实验**************************/
typedef struct _PARAM_DEMO
{
	DWORD i;
	DWORD dwRandom;
	DWORD dwData;
}PARAM_DEMO;
DWORD WINAPI ThreadProc_ZIDINGYI(LPVOID lpParameter)
{
	PARAM_DEMO *p = reinterpret_cast<PARAM_DEMO *>(lpParameter);
	printf("%d,%d,%d\n", p->i, p->dwRandom, p->dwData);
	return 0;
}
void CreateThreadDemo()
{
	PARAM_DEMO *pv1 = (PARAM_DEMO *)GlobalAlloc(GPTR, sizeof(PARAM_DEMO));
	PARAM_DEMO *pv2 = (PARAM_DEMO *)GlobalAlloc(GPTR, sizeof(PARAM_DEMO));
	pv1->i = 100;
	pv1->dwRandom = 101;
	pv1->dwData = 102;
	pv2->i = 200;
	pv2->dwRandom = 201;
	pv2->dwData = 202;

	DWORD tid1, tid2;
	HANDLE h_Thread1 = CreateThread(
		NULL,
		0,
		ThreadProc_ZIDINGYI,
		pv1,
		0,
		&tid1
	);

	HANDLE h_Thread2 = CreateThread(
		NULL,
		0,
		ThreadProc_ZIDINGYI,
		pv2,
		0,
		&tid2
	);
	WaitForSingleObject(h_Thread1, INFINITE);
	WaitForSingleObject(h_Thread2, INFINITE);
}
/************************************************************/

/****************挂起、恢复、切换、终止线程******************/
DWORD WINAPI ThreadProc(LPVOID lpParameter)
{
	LPDWORD pData = reinterpret_cast<LPDWORD>(lpParameter);
	int i = 0;
	for (i = 0; i < 50; i++)
	{
		Sleep(100);
		printf("TID = %u, \t Parameters = %u\t i = %u\n", GetCurrentThread(), *pData, i);
	}
	ExitThread(i);
	return 0;
}

void OperateThread()
{
	DWORD dwData1 = 1;
	DWORD dwData2 = 2;
	DWORD dwThreadId[2];
	HANDLE hThread[2];

	hThread[0] = CreateThread(NULL, 0, ThreadProc, &dwData1,
		CREATE_SUSPENDED,	//创建之后挂起进程
		&dwThreadId[0]);

	hThread[1] = CreateThread(NULL, 0, ThreadProc, &dwData2,
		0,		//默认标志位，创建后立即执行
		&dwThreadId[1]);

	Sleep(200);
	ResumeThread(hThread[0]);	//恢复线程0
	Sleep(300);
	SuspendThread(hThread[1]);	//暂停线程1
	
	Sleep(2000);
	TerminateThread(hThread[0], 0);//终止线程0
	ResumeThread(hThread[1]);

	WaitForMultipleObjects(2, hThread, TRUE, INFINITE);
	CloseHandle(hThread[0]);
	CloseHandle(hThread[1]);
	getchar();
}
/************************************************************/

void TestThreadDemo()
{
	//CreateThreadDemo();
	OperateThread();

}