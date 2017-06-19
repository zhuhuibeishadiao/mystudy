/*!
 * \file MsgPipe.cpp
 *
 * \author FrogGod
 * \date 五月 2017
 *
 * 命名管道实现进程数据交互。
 * 命名管道服务器使用CreateNamedPipe(pszName为"\\\\.\\pipe\\pipename");
 * 命名管道客户端使用Createfile(pszName为"\\\\servername\\pipe\\pipename");
 */

#include "stdafx.h"
#include <windows.h>
#include <iostream>
#include <string>

void run_client();
void run_server();
HANDLE CreateNewPipe(LPCTSTR lpszName);
void ReadPipeData(HANDLE h_PipeHandle);
void WritePipeData(HANDLE h_PipeHandle);

#define MY_MUTEX _T("467F8B22-5984-4A21-87A3-6AC6681D32C6")
const TCHAR g_PipeReadName[] = _T("\\\\.\\pipe\\Pipe_Read_5DAC42BD-A2E0-4A5A-A2A4-CB0A3F37E9C5");
const TCHAR g_PipeWirteName[] = _T("\\\\.\\pipe\\Pipe_Write_E97B0232-7C5E-4EB1-93AF-71727E5CA4EE");

int main()
{
	auto h_Mutex = CreateMutex(NULL, FALSE, MY_MUTEX);	//管道互斥量
	if (h_Mutex)
	{
		if (GetLastError() == ERROR_ALREADY_EXISTS)
		{
			run_client();
		}
		else
		{
			run_server();
		}
	}
    return 0;
}

void run_server()
{
	auto h_WritePipe = CreateNewPipe(g_PipeWirteName);
	std::cout << "创建WritePipe OK" << std::endl;
	auto h_ReadPipe = CreateNewPipe(g_PipeReadName);
	std::cout << "创建ReadPipe OK" << std::endl;
	
	while (true)
	{
		//pipe服务端应用程序代码
		WritePipeData(h_WritePipe);
		ReadPipeData(h_ReadPipe);
	}
}

void run_client()
{
	if (!WaitNamedPipe(g_PipeWirteName, NMPWAIT_WAIT_FOREVER))
	{
		std::cout << "Write命名管道实例不存在 ..." << std::endl;
		return;
	}
	//客户端读是服务器写
	auto h_ReadNamedPipe = CreateFile(g_PipeWirteName, GENERIC_READ, 0, NULL,
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (h_ReadNamedPipe == INVALID_HANDLE_VALUE)
	{
		std::cout << "打开命名管道失败 ..." << std::endl;
		return;
	}

	//等待READ口创建才能链接！
	Sleep(2000);

	//客户端写是服务器读
	if (!WaitNamedPipe(g_PipeReadName, NMPWAIT_WAIT_FOREVER))
	{
		std::cout << "Read命名管道实例不存在 ..." << std::endl;
		return;
	}
	auto h_WriteNamedPipe = CreateFile(g_PipeReadName, GENERIC_WRITE, 0, NULL,
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (INVALID_HANDLE_VALUE == h_WriteNamedPipe)
	{
		std::cout << "打开命名管道失败 ..." << std::endl;
		return;
	}
	while (true)
	{
		//pipe客户端应用程序代码
		ReadPipeData(h_ReadNamedPipe);
		WritePipeData(h_WriteNamedPipe);
	}
}


//************************************
// Method:    CreateNewPipe
// Returns:   返回NULL 或 0说明创建失败
// Parameter: LPCTSTR lpszName
//************************************
HANDLE CreateNewPipe(LPCTSTR lpszName)
{
	auto h_NamePipe = CreateNamedPipe(lpszName, PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED,
								     0, 1, 0x4000, 0x4000, 0, NULL);
	if (h_NamePipe == INVALID_HANDLE_VALUE)
	{
		std::cout << "创建命名管道失败..." << std::endl;
		return NULL;
	}
	//添加事件以等待客户端连接命名管道
	//该事件为手动重置事件，且初始化状态为无信号状态
	auto h_Event = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (h_Event == NULL)
	{
		std::cout << "创建事件失败..." << std::endl;
		return NULL;
	}

	//由于创建时，使用FILE_FLAG_OVERLAPPED，所以在调用cConnectNamedPipe必须有ovlap
	OVERLAPPED ovlap = { 0 };
	memset(&ovlap, 0, sizeof(ovlap));
	ovlap.hEvent = h_Event;

	//等待客户端连接
	if (ConnectNamedPipe(h_NamePipe, &ovlap) == 0)
	{
		if (ERROR_IO_PENDING != GetLastError() && ERROR_PIPE_CONNECTED != GetLastError())// || ERROR_PIPE_CONNECTED != GetLastError())
		{
			CloseHandle(h_NamePipe);
			CloseHandle(h_Event);
			std::cout << "等待客户端连接失败 ..." << std::endl;
			return 0;
		}
	}

	if (WAIT_FAILED == WaitForSingleObject(h_Event, INFINITE))
	{
		CloseHandle(h_NamePipe);
		CloseHandle(h_Event);
		std::cout << "等待对象失败..." << std::endl;
		return 0;
	}
	CloseHandle(h_Event);
	return h_NamePipe;
}

void ReadPipeData(HANDLE h_PipeHandle)
{
	CHAR szStr[MAX_PATH * 4] = { 0 };
	DWORD dwRet = 0;
	auto ret = ReadFile(h_PipeHandle, szStr, sizeof(szStr), &dwRet, NULL);
	std::cout << "读取数据成功=" << szStr << std::endl;;
}

void WritePipeData(HANDLE h_PipeHandle)
{
	std::cout << "输入数据:";
	auto msg = std::string();
	std::getline(std::cin, msg);
	DWORD dwRet = 0;
	WriteFile(h_PipeHandle, msg.c_str(), msg.size(), &dwRet, NULL);
}

