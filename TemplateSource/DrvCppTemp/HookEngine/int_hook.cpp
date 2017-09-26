#include "stdafx.h"
#include "int_hook.h"
//
//extern"C"
//{
//	void breakpoint_trap();
//	PVOID old_breakpoint_handler = nullptr;
//}
//EXTERN_C DWORD64 HandleBreakPoint(PKTRAP_FRAME _ctx)
//{
//	bool bRet = false;
//	LOG_DEBUG("#BP at %p\r\n", _ctx->Rip);
//	bRet = ddk::int_hook::getInstance().onDispatch(_ctx);
//	if (bRet)
//	{
//		return 1;
//	}
//	return 0;
//}
//namespace ddk
//{
//	void init_int_hook()
//	{
//		if (old_breakpoint_handler)
//		{
//			return;
//		}
//		//ddk::idt_hook::getInstance().MsHookInt(0x03, HandleBreakPoint);
//		//ddk::idt_hook::getInstance().hookInt(0x3, (PVOID)breakpoint_trap, old_breakpoint_handler);
//	}
//
//}