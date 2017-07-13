#pragma once
//#include "stdafx.h"
//进程操作
namespace usr::util
{
	class Process {
	private:
		HANDLE _ProcessHandle;
		DWORD_PTR ProcessId;
	public:
		Process() { _ProcessHandle = nullptr; ProcessId = GetCurrentProcessId(); };
		Process(DWORD_PTR _ProcessId) {
			ProcessId = _ProcessId;
			open_handle();
		};
		~Process() {
			if (_ProcessHandle)
			{
				ntdll::ZwClose(_ProcessHandle);
				_ProcessHandle = nullptr;
			}
		};
	public:
		Process & operator=(const Process &_Proc) 
		{
			this->_ProcessHandle = nullptr;
			this->ProcessId = _Proc.ProcessId;
			this->open_handle();
			return *this;
		}
		HANDLE get() {
			return _ProcessHandle;
		}
		void kill()
		{
			//TODO::JOB方式
			//ZwCreateJobObject()
			//ZwAssignProcessToJobObject()
			//ZwTerminateJobObject()
			//ZwClose
			//简单粗暴
			if (_ProcessHandle)
			{
				ntdll::ZwTerminateProcess(_ProcessHandle,0);
			}
		}
	private:
		void open_handle()
		{
			get_all_privilege();
			using namespace ntdll;
			HANDLE _handle = nullptr;
			ntdll::OBJECT_ATTRIBUTES oa = {};
			ntdll::CLIENT_ID cid = {};
			cid.UniqueProcess = (HANDLE)this->ProcessId;
			InitializeObjectAttributes(&oa, nullptr, OBJ_INHERIT
				|OBJ_CASE_INSENSITIVE, nullptr,nullptr);
			auto ns = ZwOpenProcess(&_handle, PROCESS_ALL_ACCESS,
				&oa, &cid);
			if (ns<0)
			{
				DebugBreak();
			}
			_ProcessHandle = _handle;
		}
	};
}