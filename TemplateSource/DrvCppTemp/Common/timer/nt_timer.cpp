#include "stdafx.h"
namespace ddk
{
	VOID _launch_callback_timer(
		PKDPC Dpc,
		PVOID DeferredContext,
		PVOID SystemArgument1,
		PVOID SystemArgument2)
	{
		//LOG_DEBUG("_launch_callback\r\n");
		UNREFERENCED_PARAMETER(Dpc);
		UNREFERENCED_PARAMETER(SystemArgument1);
		UNREFERENCED_PARAMETER(SystemArgument2);
		auto p_this = reinterpret_cast<ddk::nt_timer*>(DeferredContext);
		__try
		{
			//LOG_DEBUG("do timer\r\n");
			p_this->timer_function();
			//p_this->_Release();
		}
		__except (1)
		{
			//LOG_DEBUG("callback failed\r\n");
		}
	};
};