#pragma once
#include <windows.h>
enum ProcessType
{
	ParentProcess = 0,
	ChildProcess = 1
};

class NoNamePipe
{
public:
	NoNamePipe(ProcessType type);
	~NoNamePipe();
public:
	bool InitNoNamePipe();
	bool WriteToPipe(const char* buf, DWORD& len);
	bool ReadFromPipe(char* buf, DWORD& len);
	bool StartProcess(LPWSTR szCmdLine);
protected:
	bool CreateNoNamePipe();
	bool GetNoNamePipe();
private:
	ProcessType m_type;
	HANDLE m_hPipeRead;
	HANDLE m_hPipeWrite;
	SECURITY_ATTRIBUTES m_sa;
	STARTUPINFO m_sInfo;
	PROCESS_INFORMATION m_proInfo;
};

