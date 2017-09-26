#include "stdafx.h"
#include "idt_hook.h"


EXTERN_C DWORD64 DispatchIdt(PKTRAP_FRAME pKFrame,PVOID _Context)
{
	return ddk::idt_hook::getInstance().onDispatch(pKFrame,_Context);
}
