/*!
* \file MsgFile.cpp
*
* \author FrogGod
* \date 五月 2017
*
* 文件实现进程数据交互。
* 此项目用事件机制作为桥梁，一个进程写文件，一个读文件来实现进程间数据交换。 
* 写文件的进程用SetEvent通知读文件的进程，有新东西了。
*/

/************************************************************************
* 总结                                                                      
* 1.用GUID创建各种资源的名字，用宏代替，可以避免冲突
* 2.调用std::string.c_str()可以将string转换为c语言字符数组
* 3.清空文件，可以在使用完之后，再用trucate_exsiting打开，就可以了
* 4.c++接受用户输入std::getline(std::cin, msg);
************************************************************************/

#include "stdafx.h"
#include <windows.h>
#include <iostream>
#include <string>

#define LOCK_EVENT __T("{2E3A242A-B469-415A-B569-F54476075AC4}")
int main()
{
	auto msg = std::string("msg initial");
	auto h_Lock = CreateEvent(NULL, FALSE, FALSE, LOCK_EVENT);//<--Event同步大法

	//教程里的体检是 h_lock || h_Lock == INVALID_HANDLE_VALUE这里我把后面的去掉
	//理由是查msdn的方法
	if (h_Lock == NULL)
	{
		std::cout << "Create Event Error %d:" << GetLastError() << std::endl;
		return -1;
	}

	//事件已存在，说明之前有人创建了
	if (GetLastError() == ERROR_ALREADY_EXISTS)
	{
		
		while (true)
		{
			std::cout << "string from another process: ";
			WaitForSingleObject(h_Lock, INFINITE);
			auto h_File = CreateFile(_T("Share.dat"), GENERIC_READ | GENERIC_WRITE,
									0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
			if (h_File == INVALID_HANDLE_VALUE)
			{
				std::cout << "Create File Error %d" << GetLastError() << std::endl;
				return -1;
			}
			DWORD dwRet;
			char str[MAX_PATH] = {};
			ReadFile(h_File, str, MAX_PATH, &dwRet, NULL);
			msg = std::string(str);
			std::cout << msg << std::endl;
			CloseHandle(h_File);

			h_File = CreateFile(_T("Share.dat"), GENERIC_READ | GENERIC_WRITE,
				0, NULL, TRUNCATE_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
			CloseHandle(h_File);
		}
	}
	else//事件创建成功
	{

		while (true)
		{
			DWORD dwRet = 0;
			std::cout << "input your string for another :";
			std::getline(std::cin, msg);
			auto h_File = CreateFile(_T("Share.dat"),GENERIC_READ | GENERIC_WRITE,
									0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
			if (h_File == INVALID_HANDLE_VALUE)
			{
				std::cout << "Create File Error: "<< std::hex << GetLastError() << std::endl;
				return -1;
			}
			WriteFile(h_File, msg.c_str(), msg.size(), &dwRet, NULL);
			CloseHandle(h_File);
			SetEvent(h_Lock);
		}
	}

	return 0;
}

