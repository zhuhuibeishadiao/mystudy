#include "stdafx.h"
#include "NoNamePipe.h"


NoNamePipe::NoNamePipe(ProcessType type):m_hPipeRead(NULL), m_hPipeWrite(NULL),m_type(type)
{}


NoNamePipe::~NoNamePipe()
{
	CloseHandle(m_hPipeWrite);
	CloseHandle(m_hPipeRead);
}

bool NoNamePipe::InitNoNamePipe()
{
	bool bRet;
	switch (m_type)
	{
	case ParentProcess:
		bRet = CreateNoNamePipe();
		break;
	case ChildProcess:
		bRet = GetNoNamePipe();
		break;
	default:
		break;
	}
	return bRet;
}

bool NoNamePipe::WriteToPipe(const char * buf, DWORD& len)
{
	DWORD dwRet;
	bool bSuc = WriteFile(m_hPipeWrite, buf, len, &dwRet, NULL);
	len = dwRet;
	return bSuc;
}

bool NoNamePipe::ReadFromPipe(char * buf, DWORD& len)
{
	DWORD dwRet;
	bool bSuc = ReadFile(m_hPipeRead, buf, len, &dwRet, NULL);
	len = dwRet;
	return bSuc;
}

bool NoNamePipe::StartProcess(LPWSTR szCmdLine)
{
	ZeroMemory(&m_proInfo, sizeof(PROCESS_INFORMATION));
	ZeroMemory(&m_sInfo, sizeof(STARTUPINFO));
	m_sInfo.cb = sizeof(STARTUPINFO);
	m_sInfo.hStdError = m_hPipeWrite;
	m_sInfo.hStdOutput = m_hPipeWrite;
	m_sInfo.hStdInput = m_hPipeRead;
	m_sInfo.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
	bool bSuccess = CreateProcess(NULL, szCmdLine, NULL, NULL, TRUE,
		0, NULL, NULL, &m_sInfo, &m_proInfo);
	if (!bSuccess)
		return false;
	CloseHandle(m_proInfo.hProcess);
	CloseHandle(m_proInfo.hThread);
	return true ;
}

bool NoNamePipe::CreateNoNamePipe()
{
	m_sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	m_sa.bInheritHandle = true;
	m_sa.lpSecurityDescriptor = NULL;

	if (!CreatePipe(&m_hPipeRead, &m_hPipeWrite, &m_sa, 0))
		return false;
	
	if (!SetHandleInformation(m_hPipeRead, HANDLE_FLAG_INHERIT, 0)
		&& !SetHandleInformation(m_hPipeWrite, HANDLE_FLAG_INHERIT, 0))
		return false;
	return true;
}

bool NoNamePipe::GetNoNamePipe()
{
	m_hPipeRead = GetStdHandle(STD_OUTPUT_HANDLE);
	m_hPipeWrite = GetStdHandle(STD_INPUT_HANDLE);
	if ((m_hPipeRead == INVALID_HANDLE_VALUE) ||
		(m_hPipeWrite == INVALID_HANDLE_VALUE))
		return false;
	else
		return true;
	return false;
}
