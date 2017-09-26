#include "stdafx.h"

namespace ddk::util
{
	using idt_context = struct
	{
		ULONG vec;
		PVOID pfn;
	};
	NTSTATUS EachProcess(PVOID _context)
	{
		auto pCtx = reinterpret_cast<idt_context*>(_context);
		if (pCtx)
		{
			KDESCRIPTOR Idtr = {};
			KIRQL OldIrql;
			PKIDTENTRY64 pIdt = nullptr;
			PKIDTENTRY64 pVector = nullptr;
			auto pFunc = pCtx->pfn;
			auto LowPart = (ULONG32)((ULONGLONG)pFunc);
			__sidt(&Idtr.Limit);
			__try
			{
				pIdt = (PKIDTENTRY64)Idtr.Base;
				pVector = &pIdt[pCtx->vec];
			}
			__except (1)
			{
				return STATUS_UNSUCCESSFUL;
			}
			KeRaiseIrql(HIGH_LEVEL, &OldIrql);
			_disable();
			__writecr0(__readcr0() & (~(0x10000)));
			pVector->OffsetHigh = (ULONG32)((ULONGLONG)pFunc >> 32);
			pVector->OffsetLow = (UINT16)LowPart;
			pVector->OffsetMiddle = (UINT16)(LowPart >> 16);
			__writecr0(__readcr0() ^ 0x10000);
			_enable();
			KeLowerIrql(OldIrql);
		}
		return STATUS_SUCCESS;
	}
	bool set_idt_handler(ULONG _vec, PVOID _pfn)
	{
		//需要遍历全部Cpu
		idt_context ctx = {};
		ctx.pfn = _pfn;
		ctx.vec = _vec;
		auto ns = ddk::util::ForEachProcessors(
			std::bind(&ddk::util::EachProcess,
				std::placeholders::_1), &ctx);
		if (NT_SUCCESS(ns))
		{
			return true;
		}
		return false;
	}
	PVOID get_idt_handler(ULONG _vec)
	{
		KDESCRIPTOR Idtr = {};
		__sidt(&Idtr.Limit);
		auto pIdt = (PKIDTENTRY64)Idtr.Base;
		auto Entry = pIdt[_vec];
		auto pRet = PVOID(((ULONG_PTR)Entry.OffsetHigh << 32) | ((ULONG_PTR)Entry.OffsetMiddle << 16 | ((ULONG_PTR)Entry.OffsetLow)));
		return pRet;
	}
}