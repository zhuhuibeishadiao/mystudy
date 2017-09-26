#pragma once
#include "stdafx.h"
#include "idt_hook.h"
#include "BaseHook.h"
//hook INT3
//
namespace ddk
{
	//void init_int_hook();
	class int_hook :public Singleton<int_hook>
	{
	public:
		int_hook()
		{
			//hook int 0xXX
			//init_int_hook();
			ddk::idt_hook::getInstance().MsHookInt(0x03, 
				std::bind(&ddk::int_hook::onDispatch, this, std::placeholders::_1),false);
		}
		~int_hook()
		{
			_intlock.wait_for_release();
			for (auto _hook:_old)
			{
				::Detours::Internal::AtomicCopy4X8((uint8_t*)_hook.Target,
					(uint8_t*)_hook.SavedCode, 1);
				ExFreePool(_hook.jmpOld);
			}
		}
	private:
		using OLD_SAVE = struct 
		{
			PVOID Target;
			BYTE SavedCode[8];
			PVOID jmpOld;
		};
	public:
		bool hook(PVOID Target, PVOID NewAddress, PVOID &OldAddress)
		{
			//首先Make Old
			//首先从Target里复制8字节出来
			OLD_SAVE _save = {};
			BYTE PatchCode[] = { 0xCC,0xCC,0xCC,0xCC };
			RtlCopyMemory(_save.SavedCode, Target, 8);
			_save.Target = Target;
			auto backup_size = ::Detours::Internal::GetCodeBackupSize((uint64_t)Target, 1);
			if (!backup_size)
			{
				return false;
			}
			PVOID backupCode = ExAllocatePool(NonPagedPool,backup_size + 0x100);
			if (!backupCode)
			{
				return false;
			}
			RtlFillBytes(backupCode, backup_size + 0x100, 0x90);
			RtlCopyMemory(backupCode, Target, backup_size);

			auto BackCode = (ULONGLONG)backupCode + backup_size;
			::AsmGen BackGen(BackCode, ASMGEN_64);
			BackGen.AddCode("push 0x%X", ((uint64_t)Target+backup_size) & 0xFFFFFFFF);
			BackGen.AddCode("mov dword ptr ss:[rsp+0x4], 0x%X", ((uint64_t)Target + backup_size) >> 32);
			BackGen.AddCode("ret");
			BackGen.WriteStreamTo((BYTE *)BackCode);

			OldAddress = backupCode;
			_save.jmpOld = backupCode;
			_red_map[(ULONG64)Target] = (ULONG64)NewAddress;
			::Detours::Internal::AtomicCopy4X8((uint8_t*)Target, (uint8_t*)PatchCode, 1);
			_old.push_back(_save);
			return true;
		}
		DWORD64 onDispatch(PKTRAP_FRAME _pContext)
		{
			_intlock.only_acquire();
			auto exit1 = std::experimental::make_scope_exit([&]() {
				_intlock.release();
			});
			LOG_DEBUG("#BP at %p\r\n", _pContext->Rip);
			auto _addr = _pContext->Rip - 1;
			auto pfind = _red_map.find(_addr);
			if (pfind != _red_map.end())
			{
				_pContext->Rip = pfind->second;
				return 1;
			}
			return 0;
		}
	private:
		std::map<ULONG64, ULONG64>_red_map;
		std::vector<OLD_SAVE> _old;
		ddk::nt_lock _intlock;
	};
}