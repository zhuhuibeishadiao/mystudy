#include "stdafx.h"
//frog

/**************使用Event进行同步**********************/
CHAR szFoo[MAX_PATH] = "草泥马";
HANDLE hEvent = NULL;
DWORD WINAPI Demo1Proc(LPVOID lpParameter)
{
	if (!WaitForSingleObject(hEvent, INFINITE))
	{
		std::cout << "我是线程函数,我收到事件触发了，数据为：";
		std::cout << szFoo << std::endl;;
	}
	return 0;
}

void EventDemo1()
{
	hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	HANDLE hThread = CreateThread(NULL, 0, Demo1Proc, NULL, 0, NULL);

	CopyMemory(szFoo, "不要草泥马啦", sizeof("不要草泥马啦"));
	std::cout << "数据准备完毕0.5秒之后signal事件" << std::endl;
	Sleep(500);
	SetEvent(hEvent);
	Sleep(200);
}
/*****************************************************/

void TestEvent()
{
	EventDemo1();
}
