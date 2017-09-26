#include "stdafx.h"

namespace ddk
{
	VOID _launch_callback_dpc(
		PKDPC Dpc,
		PVOID DeferredContext,
		PVOID SystemArgument1,
		PVOID SystemArgument2)
	{
		//LOG_DEBUG("_launch_callback\r\n");
		//KeLowerIrql(PASSIVE_LEVEL);
		UNREFERENCED_PARAMETER(Dpc);
		UNREFERENCED_PARAMETER(SystemArgument1);
		UNREFERENCED_PARAMETER(SystemArgument2);
		auto p_this = reinterpret_cast<ddk::_ddkPad_dpc*>(DeferredContext);
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
	}
};