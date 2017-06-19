#include "stdafx.h"

#define THREAD_NUM 3
int g_num = 0;
int g_num1 = 0;
HANDLE hThreads[THREAD_NUM];
HANDLE gMutex = NULL;
/*****************不使用互斥对全局做加*********************/
DWORD WINAPI DoAdd(LPVOID lpParam)
{
	for (auto i = 0; i < 100000; i++)
	{
		g_num++;
	}
	return 1;
}

void AddWithoutMutex()
{
	for (auto i = 0; i < THREAD_NUM; i++)
	{
		hThreads[i] = CreateThread(NULL, 0, DoAdd, &i, 0, NULL);
	}
	WaitForMultipleObjects(THREAD_NUM, hThreads, TRUE, INFINITE);
	std::cout << "结果：" << g_num << "(不正确，未使用互斥量)" << std::endl;
	getchar();
}
/**********************************************************/

/********************互斥量的线程同步*********************/
DWORD WINAPI DoAdd1(LPVOID lpParam)
{
	//调用wait函数，会把mutex变为线程拥有，令其他线程等待
	//若mutex已被其他线程拥有，则等待
	if (!WaitForSingleObject(gMutex, INFINITE))
	{
		for (auto i = 0; i < 100000; i++)
		{
			g_num1++;
		}
		ReleaseMutex(gMutex);
	}
	return 1;
}

void AddWithMutex()
{
	gMutex = CreateMutex(NULL, TRUE, NULL);

	for (auto i = 0; i < THREAD_NUM; i++)
	{
		hThreads[i] = CreateThread(NULL, 0, DoAdd1, NULL, 0, NULL);
	}
	WaitForMultipleObjects(THREAD_NUM, hThreads, TRUE, INFINITE);
	std::cout << "结果：" << g_num1 << std::endl;
	getchar();
}
/*********************************************************/

void TestMutexDemo()
{
	AddWithoutMutex();
	AddWithMutex();
}