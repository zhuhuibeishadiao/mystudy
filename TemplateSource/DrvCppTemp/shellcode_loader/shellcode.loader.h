#pragma once
#include "stdafx.h"



namespace ddk::shellcode
{
	typedef struct _SHELL_DPC_
	{
		KDPC dpc;
		DWORD64 Key;
		KTIMER _Timer;
	}SHELL_DPC, *PSHELL_DPC;

	typedef struct _SHELL_CONTEXT
	{
		UCHAR CmpAppendDllSection[192];
		ULONG32 unknown;
		ULONG32 ContextSizeInQWord;
		ULONG64 ExAcquireResourceSharedLite;
		decltype(&ExAllocatePoolWithTag) _ExAllocatePoolWithTag;
		ULONG64 ExFreePool;
		ULONG64 ExMapHandleToPointer;
		ULONG64 ExQueueWorkItem;
		ULONG64 ExReleaseResourceLite;
		ULONG64 ExUnlockHandleTableEntry;
		ULONG64 ExfAcquirePushLockExclusive;
		ULONG64 ExfReleasePushLockExclusive;
		ULONG64 KeAcquireInStackQueuedSpinLockAtDpcLevel;
		ULONG64 ExAcquireSpinLockShared;
		ULONG64 KeBugCheckEx;
		ULONG64 KeDelayExecutionThread;
		ULONG64 KeEnterCriticalRegionThread;
		ULONG64 KeLeaveCriticalRegion;
		ULONG64 KeEnterGuardedRegion;
		ULONG64 KeLeaveGuardedRegion;
		ULONG64 KeReleaseInStackQueuedSpinLockFromDpcLevel;
		ULONG64 ExReleaseSpinLockShared;
		ULONG64 KeRevertToUserAffinityThread;
		ULONG64 KeProcessorGroupAffinity;
		ULONG64 KeSetSystemGroupAffinityThread;
		decltype(&KeSetTimer) _KeSetTimer;
		ULONG64 LdrResFindResource;
		ULONG64 MmDbgCopyMemory;
		ULONG64 ObfDereferenceObject;
		ULONG64 ObReferenceObjectByName;
		ULONG64 RtlAssert;
		ULONG64 RtlImageDirectoryEntryToData;
		ULONG64 RtlImageNtHeader;
		ULONG64 RtlLookupFunctionTable;
		ULONG64 RtlSectionTableFromVirtualAddress;
		ULONG64 DbgPrint;
		ULONG64 DbgPrintEx;
		ULONG64 KiProcessListHead;
		ULONG64 KiProcessListLock;
		ULONG64 unknown1;
		ULONG64 PsActiveProcessHead;
		ULONG64 PsInvertedFunctionTable;
		ULONG64 PsLoadedModuleList;
		ULONG64 PsLoadedModuleResource;
		ULONG64 PsLoadedModuleSpinLock;
		ULONG64 PspActiveProcessLock;
		ULONG64 PspCidTable;
		ULONG64 SwapContext;
		ULONG64 EnlightenedSwapContext;
		ULONG64 unknown2;
		ULONG64 unknown3;//0x240?
		ULONG64 unknown4;//0x248
		ULONG64 workerRoutine;//0x250
		ULONG64 workerQueueContext;//0x258
		ULONG64 unknown5;//0x260
		ULONG64 Prcb;//0x268
		ULONG64 PageBase;//0x270
		ULONG64 DcpRoutineToBeScheduled;//0x278
		ULONG32 unknown6;//0x280
		ULONG32 unknown7;//0x284
		ULONG32 offsetToPg_SelfValidation;//0x288
		ULONG32 offsetToRtlLookupFunctionEntryEx;//0x28C
		ULONG32 offsetToFsRtlUninitializeSmallMcb;//0x290
		ULONG32 unknown8;//0x294
		ULONG64 field_298;
		ULONG64 field_2A0;
		ULONG64 field_2A8;
		ULONG64 field_2B0;
		ULONG64 field_2B8;
		ULONG64 field_2C0;
		ULONG64 field_2C8;
		ULONG64 field_2D0;
		ULONG64 field_2D8;
		ULONG64 field_2E0;
		ULONG64 field_2E8;
		ULONG64 field_2F0;
		ULONG64 field_2F8;
		ULONG64 field_300;
		ULONG64 isErroFound;
		ULONG64 bugChkParam1;
		ULONG64 bugChkParam2;
		ULONG64 bugChkParam4Type;
		ULONG64 bugChkParam3;
		ULONG64 field_330;
		BYTE    retn[0x100];
		ULONG _mySeed;
		PSHELL_DPC _ShellDpc;
		PVOID old_context;
		DWORD64 _offset_shellcode;
		DWORD64 _offset_EncryptShellCode;
		DWORD64 _offset_EncryptShellContext;
		DWORD64 _offset_ScTimer;
		DWORD64 _TimerSize;
		DWORD64 _sizeAll;
		decltype(&KeCancelTimer) _KeCancelTimer;
		decltype(&ExFreePoolWithTag) _ExFreePoolWithTag;
		DWORD64 _EncryptShellContext;
		PKDEFERRED_ROUTINE _ScTimer;
		decltype(&KeInitializeTimer) _KeInitializeTimer;
		decltype(&KeInitializeDpc) _KeInitializeDpc;
		decltype(&memcpy) _memcpy;
		decltype(&RtlRandom) _RtlRandom;
		///这里为某个大阴谋准备的
		DWORD64 _pRelocateDriver;
		DWORD64 _DrvNewBase;
		DWORD64 _pNtosBase;
		decltype(&MmGetSystemRoutineAddress) _MmGetSystemRoutineAddress;
		decltype(&RtlInitUnicodeString) _RtlInitUnicodeString;
	}ShellContext, *PSHELL_CONTEXT;

	bool run_shellcode(PVOID _shellcode, size_t _shellcode_size);
};