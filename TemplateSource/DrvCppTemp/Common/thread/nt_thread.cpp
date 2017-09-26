#include "stdafx.h"
namespace ddk
{
	VOID _launch_callback(IN PVOID _Data)
	{
		//LOG_DEBUG("_launch_callback\r\n");
		//处理线程隐藏
#if !defined(DBG)
		auto thread_ptr = reinterpret_cast<PULONG_PTR>(PsGetCurrentThread());
		__try
		{
			for (auto i = 0; i < 0x280; i++)
			{
				if (thread_ptr[i] == (ULONG_PTR)_launch_callback)
				{
					thread_ptr[i] = (ULONG_PTR)((PBYTE)&RtlInitUnicodeString + 0x230);
					//break;
				}
			}
		}
		__except (1)
		{

		}
#endif
		auto p_this = reinterpret_cast<ddk::_ddkPad*>(_Data);
		__try
		{
			//LOG_DEBUG("do callback\r\n");
			p_this->_Go();
			p_this->_Release();
		}
		__except (1)
		{
			LOG_DEBUG("callback failed\r\n");
		}
		PsTerminateSystemThread(STATUS_SUCCESS);
	}
};