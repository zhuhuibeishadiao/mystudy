#pragma once
#include "stdafx.h"
extern "C"
{
	void InterruptRoutine();
	void InterruptRoutinePF();
	DWORD64 DispatchIdt(PKTRAP_FRAME pKFrame, PVOID _Context);
};
namespace ddk
{
	//NTSTATUS HalGetVectorInput(ULONG Vector);
	using idt_callback = std::function<DWORD64(PKTRAP_FRAME)>;
	typedef struct _RBP_FRAME_
	{
		DWORD64 VecNumber;
		DWORD64 Old_Vector;
		DWORD64 DispatchAddr;
	}RBP_FRAME, *PRBP_FRAME;

	class idt_hook :public Singleton<idt_hook>
	{
	public:
		idt_hook() {

		}
		~idt_hook() {
			_intlock.wait_for_release();
			for (auto _hook:_hooklist2)
			{
				ddk::util::set_idt_handler(_hook.first, _hook.second);
			}
			for (auto _free:_freelist)
			{
				ExFreePool(_free);
			}
		}
	public:
		bool hookInt(ULONG _vecNum, PVOID _vecpfn, PVOID &_oldpfn)
		{
			//hook方法1
			__try
			{ 
				_oldpfn = ddk::util::get_idt_handler(_vecNum);
			}
			__except (1)
			{
				return false;
			}
			if (_hooklist2.find(_vecNum)!=_hooklist2.end())
			{
				return false;
			}
			auto bret= ddk::util::set_idt_handler(_vecNum, _vecpfn);
			if (bret)
			{
				_hooklist2[_vecNum] = _oldpfn;
			}
			return bret;
		}
		bool MsHookInt(ULONG _vecNum,idt_callback _pfnCallBack,bool has_errorcode)
		{
			//微软风格的inthook方式，碉堡
			auto dispatchRoutine = (PVOID)InterruptRoutine;
			if (has_errorcode)
			{
				dispatchRoutine = (PVOID)InterruptRoutinePF;
			}
			//不能hook 已经hook过的
			if (_idtdispatch.find(_vecNum) != _idtdispatch.end())
			{
				return false;
			}
			auto pIdtNewShellcode = ExAllocatePool(NonPagedPool, PAGE_SIZE);
			if (!pIdtNewShellcode)
			{
				return false;
			}
			LOG_DEBUG("hook addr %p\r\n", pIdtNewShellcode);
			auto exit1 = std::experimental::make_scope_exit([&]() {
				ExFreePool(pIdtNewShellcode);
			});
			auto _shellcodex = reinterpret_cast<PUCHAR>(pIdtNewShellcode);
			RtlFillBytes(pIdtNewShellcode, 0x300, 0x90);
			RtlCopyMemory(_shellcodex, dispatchRoutine, 0x300);
			auto _oldpfn = ddk::util::get_idt_handler(_vecNum);
			auto pContext = reinterpret_cast<PRBP_FRAME>((PUCHAR)pIdtNewShellcode + 0x2);
			pContext->VecNumber = _vecNum;
			pContext->Old_Vector = (DWORD64)_oldpfn;
			pContext->DispatchAddr = (DWORD64)DispatchIdt;
			_idtdispatch[_vecNum] = _pfnCallBack;
			if (ddk::util::set_idt_handler(_vecNum,pIdtNewShellcode))
			{
				exit1.release();
				_hooklist2[_vecNum] = _oldpfn;
				_freelist.push_back(pIdtNewShellcode);
				return true;
			}
			return false;
		}
		bool is_hookpf(PVOID va)
		{
			auto pde = ddk::mem_util::UtilpAddressToPde(va);
			auto pte = ddk::mem_util::UtilpAddressToPte(va);
			for (auto _alloc:_freelist)
			{
				auto mypde = ddk::mem_util::UtilpAddressToPde(_alloc);
				auto mypte = ddk::mem_util::UtilpAddressToPte(_alloc);
				if (pde->LargePage)
				{
					//我都是nonpagedpool正常都是Larged
					if(pde==mypde)
						return true;
				}
				else
				{
					if (!mypde->LargePage && pte && mypte
						&& mypte==pte)
					{
						return true;
					}
				}
			}
			return false;
		}
	public:
		void acquire()
		{
			_intlock.only_acquire();
		}
		void release()
		{
			_intlock.release();
		}
		DWORD64 onDispatch(PKTRAP_FRAME _kframe,PVOID _Context)
		{
			DWORD64 dwRet = 0;
			ULONG VecNm = 0;
			acquire();
			auto pContext = (PRBP_FRAME)(_Context);
			VecNm = (ULONG)pContext->VecNumber;
			auto pfn = _idtdispatch[VecNm];
			if (pfn)
			{
				dwRet = pfn(_kframe);
			}
			release();
			return dwRet;
		}
	private:
		//std::vector<PKINTERRUPT> _hooklist1;
		std::map<ULONG, PVOID>_hooklist2;
		std::map<ULONG, idt_callback> _idtdispatch;
		std::vector<PVOID>_freelist;
		ddk::nt_lock _intlock;
	};
};