#include "stdafx.h"

VOID GetAllPrivilege();

//远程线程注入
extern bool inject_remote_thread(DWORD dwPid, LPCWSTR lpszDllPath);
extern BOOL inject_apc_dll(DWORD dwProcessId, LPCWSTR lpszDllFilePath);//目前代码值使用32位
extern BOOL inject_context_dll(DWORD dwProcessId, LPCWSTR lpszDllFilePath);
int main()
{
	GetAllPrivilege();
	//inject_remote_thread(1000, L"C:\\Users\\killvxk\\Desktop\\mydll32.dll");//可以
	//inject_remote_thread(5148, L"C:\\Users\\killvxk\\Desktop\\mydll64.dll");//可以
	//inject_apc_dll(3984, L"C:\\Users\\killvxk\\Desktop\\mydll32.dll");//apc注入尚未成功了
	//inject_context_dll(3984, L"C:\\Users\\killvxk\\Desktop\\mydll32.dll");//context注入，成功了
	
	return 0;
}


VOID GetAllPrivilege()
{
	for (USHORT i = 0; i < 0x100; i++)
	{
		BOOLEAN Old;
		RtlAdjustPrivilege(i, TRUE, FALSE, &Old);
	}
}

