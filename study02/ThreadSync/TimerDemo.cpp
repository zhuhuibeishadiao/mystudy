#include "stdafx.h"


#define ONE_SECOND 10000000	//一秒

/**************************可等待计时器应用*********************************/
void Demo1()
{
	auto hTimer = CreateWaitableTimer(NULL, FALSE, NULL);
	LARGE_INTEGER liDueTime;
	liDueTime.QuadPart = -3 * ONE_SECOND;
	SetWaitableTimer(hTimer, &liDueTime, 2000, NULL, NULL, FALSE);
	
	while (TRUE)
	{
		//第一次3秒后执行；
		//之后每2秒执行一次，无限循环。
		if (!WaitForSingleObject(hTimer, INFINITE))
		{
			printf("hoho\n");
		}
	}
	
}
/************************************************************************/

/*************************APC的可等待计时器******************************/
typedef struct _APC_PROC_ARG
{
	CHAR *szText;
	DWORD dwValue;
}APC_PROC_ARG;
VOID CALLBACK TimerAPCProc(LPVOID lpArgToCompletionRoutine, DWORD  dwTimerLowValue, DWORD  dwTimerHighValue)
{
	APC_PROC_ARG *pData = (APC_PROC_ARG *)lpArgToCompletionRoutine;
	printf("Message:%s\nValue:%d\n\n", pData->szText, pData->dwValue);
	MessageBeep(MB_OK);
}
void Demo2()
{
	auto hTimer = CreateWaitableTimer(NULL, FALSE, L"MyTimer");
	APC_PROC_ARG ApcData;
	ApcData.szText = "Message to apc proc";
	ApcData.dwValue = 1;
	//INT64 qwDueTime;
	LARGE_INTEGER liDueTime;

	if (hTimer)
	{
		liDueTime.QuadPart = -5 * ONE_SECOND;
		auto bSuccess = SetWaitableTimer(
			hTimer,
			&liDueTime, //第一次触发在什么时候，可以给绝对时间，也可以给相对时间，这里是用相对时间
			2000,		//每隔多少秒触发一次
			TimerAPCProc,
			&ApcData,
			FALSE);

		for (; ApcData.dwValue <= 4; ApcData.dwValue++)
		{
			printf("开始等待了\n");
			SleepEx(INFINITE, TRUE);
		}
	}
}
/************************************************************************/
void TestTimerDemo()
{
	Demo1();
	//Demo2();
}