// WinmainConsole.cpp : 定义应用程序的入口点。
//
/*!
* \file WinmainConsole.cpp
*
* \author frogGod
* \date 五月 2017
*
*  WinMain调用console(控制台)
*/


#include "stdafx.h"
#include <stdio.h>
#include "WinmainConsole.h"


int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);
	AllocConsole();
	SetConsoleTitle(L"草泥马的");
	AttachConsole(GetCurrentProcessId());


	FILE *pFile = nullptr;
	freopen_s(&pFile, "CON", "r", stdin);
	freopen_s(&pFile, "CON", "w", stdout);
	freopen_s(&pFile, "CON", "w", stderr);
	_tprintf(L"what is this\r\n");
	MessageBox(NULL, _T("一个提示"), _T("Me"), MB_OK);
    return 0;
}


