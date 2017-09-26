#pragma once
namespace ddk
{
	//Log日志系统
	//函数LogToFile
	void LogToFileA(LPCSTR lpszFormat, ...);
	void LogToFileW(LPCWSTR lpszFormat, ...);
}