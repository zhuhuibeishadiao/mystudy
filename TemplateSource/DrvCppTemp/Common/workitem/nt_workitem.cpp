#include "stdafx.h"
namespace ddk
{
	VOID _launch_callback_WI(IN PVOID _Data)
	{
		//LOG_DEBUG("_launch_callback\r\n");
		auto p_this = reinterpret_cast<ddk::_ddkPad_WI*>(_Data);
		__try
		{
			//LOG_DEBUG("do callback\r\n");
			p_this->_Go();
			//p_this->_Release();
		}
		__except (1)
		{
			LOG_DEBUG("callback failed\r\n");
		}
	};
};