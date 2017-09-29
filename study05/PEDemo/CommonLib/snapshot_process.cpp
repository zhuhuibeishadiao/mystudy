#include "stdafx.h"

namespace usr::util::snapshot
{
	void get_snapshot_process(std::vector<process_item> &_process)
	{
		get_all_privilege();


		unsigned long cbBuffer = 0x5000;  //Initial Buffer Size
		void* Buffer = (void*)malloc(cbBuffer);
		auto exit1 = std::experimental::make_scope_exit([&]() {
			if (Buffer)
				free(Buffer);
		});
		if (Buffer == 0) return;
		bool x = false;
		bool error = false;
		while (x == false)
		{
			int ret = ntdll::NtQuerySystemInformation(ntdll::SystemExtendedProcessInformation, Buffer, cbBuffer, 0);
			if (ret < 0)
			{
				if (ret == STATUS_INFO_LENGTH_MISMATCH)
				{
					cbBuffer = cbBuffer + cbBuffer;
					free(Buffer);
					Buffer = (void*)malloc(cbBuffer);
					if (Buffer == 0) return;
					x = false;
				}
				else
				{
					x = true;
					error = true;
				}
			}
			else x = true;
		}
		if (error == false)
		{
			ntdll::SYSTEM_PROCESS_INFORMATION* p = (ntdll::SYSTEM_PROCESS_INFORMATION*)Buffer;
			while (1)
			{
#define min(a, b) (((a) < (b)) ? (a) : (b))
				WCHAR szName[MAX_PATH] = { 0 };
				RtlCopyMemory(szName, p->ImageName.Buffer, min((SIZE_T)p->ImageName.MaximumLength, 512));
				process_item item = {};
				item.ProcessId = (DWORD_PTR)p->UniqueProcessId;
				item.process_name = _tstring(szName);
				_process.push_back(item);
				if (p->NextEntryOffset == 0) break;
				p = (ntdll::SYSTEM_PROCESS_INFORMATION*)((unsigned char*)p + (p->NextEntryOffset));
			}
		}
	}
}

