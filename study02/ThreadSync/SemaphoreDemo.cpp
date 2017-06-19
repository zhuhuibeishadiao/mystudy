#include "stdafx.h"
#define  THREAD_NUM 4
HANDLE hSemaThreads[THREAD_NUM];
HANDLE gSem;

//信号量线程同步

DWORD WINAPI SemaThreadProc(LPVOID lpParam)
{
	int num = *((int *)lpParam);
	LONG oldCnts;
	for (auto i = 0; i < THREAD_NUM; i++)
	{
		if (!WaitForSingleObject(gSem, INFINITE))
		{
			std::cout << "线程" << num << "获得信号量。" << std::endl;
			Sleep(500);
			ReleaseSemaphore(gSem, 1, &oldCnts);
		}
	}
	return 1;
}


void Demo()
{
	gSem = CreateSemaphore(NULL, 3, 3, NULL);

	for (auto i = 0; i < THREAD_NUM; i++)
	{
		hSemaThreads[i] = CreateThread(NULL, 0, SemaThreadProc, &i, 0, NULL);
		Sleep(200);	//稍微停下，要不然全部创建，序号会乱掉，因为我们传的是地址
	}
	WaitForMultipleObjects(THREAD_NUM, hSemaThreads, TRUE, INFINITE);
	getchar();
}

void TestSemaphoreDemo()
{
	Demo();
}