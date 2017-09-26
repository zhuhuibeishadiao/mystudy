#include "stdafx.h"
#include "patchguard.h"
#include "..\HookEngine\AllHooks.h"

extern "C"
{
	PVOID old_debugfault_handler = nullptr;
	PVOID old_pftrap_handler = nullptr;
	DWORD64 _SystemCr3 = 0;
	void pagefault_trap();
	void debugfault_trap();
	void PF_Test();
};

void pgRetn()
{
	LOG_DEBUG("i fucked\r\n");
	return;
}

void TestPf()
{
	LOG_DEBUG("OK PF\r\n");
	return;
}
bool is_code_KiInterruptTemplate(PVOID virtual_address)
{
	// nt!KiInterruptTemplate:
	// 50              push    rax
	// 55              push    rbp
	// 488d2d67ffffff  lea     rbp, [rip-0x99]
	// ff6550          jmp     qword ptr[rbp + 50h]
	static const UCHAR kKiInterruptTemplate[] = {
		0x50, 0x55, 0x48, 0x8d, 0x2d, 0x67, 0xff, 0xff,
		0xff, 0xff, 0x65, 0x50,
	};
	return (RtlCompareMemory(virtual_address, kKiInterruptTemplate,
		sizeof(kKiInterruptTemplate)) ==
		sizeof(kKiInterruptTemplate));
}
bool tlb_load(void *v_address)
{
	DWORD a = 0;
	__try
	{
		a = *(DWORD *)v_address;
	}
	__except (1)
	{
		return false;
	}
	return true;
}
typedef struct _MTF_RECORD_
{
	BOOL bMtf;
	PVOID pfAddress;
	ULONG32 oldEflags;
	ULONG64 oldRip;
}MTF_RECORD, *PMTF_RECORD;
MTF_RECORD mtf_list[0x1000] = {};

EXTERN_C DWORD64 handle_debugfault_trap(PKTRAP_FRAME _kframe)
{
	auto cpuindex = __readgsbyte(0x52);
	auto dr6 = __readdr(6);
	//LOG_DEBUG("#DB Rip=%p\r\n", _kframe->Rip);
	auto pMtf = &mtf_list[cpuindex];
	if (pMtf->bMtf)
	{
		pMtf->bMtf = FALSE;
		auto fpAddress = pMtf->pfAddress;
		_kframe->EFlags = pMtf->oldEflags;
		ddk::mem_util::MmSetAddresssNoExecutable(fpAddress);
		__invlpg(fpAddress);
		return 1;
	}
	return 0;
}
EXTERN_C DWORD64 handle_pagefault_trap(PKTRAP_FRAME _kframe)
{
	//_kframe->ErrorCode
	PVOID fpAddress = nullptr;
	PageFaultErrorCode Ec;
	auto cpuindex = __readgsbyte(0x52);
	auto SaveCr3 = __readcr3();
	Ec.all = (ULONG32)_kframe->ErrorCode;
	fpAddress = (PVOID)_kframe->FaultAddress;
	//LOG_DEBUG("EC == %08x %p %p\r\n", Ec.all, fpAddress, _kframe->Rip);
	if (_kframe->FaultAddress == 0x0ABCDABCDui64)
	{
		_kframe->Rip = (ULONG64)TestPf;
		return 1;
	}
	if (!Ec.fields.present || Ec.fields.user)
	{
		return 0;
	}
	if (Ec.fields.fetch
		&& _kframe->Rip > (ULONG64)MmSystemRangeStart)
	{
		//LOG_DEBUG("NX %p %p\r\n", fpAddress, _kframe->Rip);
		//发生Fetch就是nx位设置了
		DWORD dwSig = *(DWORD *)fpAddress;
		if (dwSig == 0x1131482E || dwSig == 0x48513148)
		{
			//测试PG 0x1131482E 0x48513148
			_kframe->Rip = (ULONG64)pgRetn;
			return 1;
		}
		else
		{
			//#DB
			auto pMtf = &mtf_list[cpuindex];
			pMtf->oldEflags = _kframe->EFlags;
			pMtf->oldRip = _kframe->Rip;
			pMtf->bMtf = TRUE;
			pMtf->pfAddress = fpAddress;
			//设置DB
			_kframe->EFlags |= 0x100;
		}
		ddk::mem_util::MmSetAddresssExecutable(fpAddress);
		__invlpg(fpAddress);
		return 1;
	}
	return 0;
}


namespace ddk::patchguard
{
	static const ULONG64 CmpAppendDllSection_PATTERN[] =
	{
		0x085131481131482E,
		0x1851314810513148,
		0x2851314820513148,
		0x3851314830513148,
		0x4851314840513148,
		0x5851314850513148,
		0x6851314860513148,
		0x7851314870513148,
		0x4800000080913148,
		0x3148000000889131,
		0x9131480000009091,
		0xA091314800000098,
		0x00A8913148000000,
		0x0000B09131480000,
		0x000000B891314800,
		0x31000000C0913148,
		0x8BD18B48C28B4811,
		0x843148000000C48A,
		0xC8D348000000C0CA,
	};
	// Just to know the length
	C_ASSERT(sizeof(CmpAppendDllSection_PATTERN) == 0x98);

	static const UCHAR KiDpcDispatch_PATTERN[] = {
		0x48, 0x31, 0x51, 0x48, 0x48, 0x31, 0x51, 0x50, 0x48, 0x83,
		0xC1, 0x48, 0x48, 0x31, 0x51, 0x10, 0x48, 0x31, 0x51, 0x18,
		0x48, 0x31, 0x51, 0x20, 0x48, 0x31, 0x51, 0x28, 0x48, 0x31,
		0x51, 0x30, 0x48, 0x31, 0x51, 0x38, 0x48, 0x31, 0x51, 0x40,
		0x48, 0x31, 0x51, 0x48, 0x48, 0x31, 0x51, 0x50, 0x48, 0x31,
		0x51, 0x58, 0x48, 0x31, 0x51, 0x60, 0x48, 0x31, 0x51, 0x68,
		0x48, 0x31, 0x51, 0x70, 0x48, 0x31, 0x51, 0x78, 0x48, 0x31,
		0x91, 0x80, 0x00, 0x00, 0x00, 0x48, 0x31, 0x91, 0x88, 0x00,
		0x00, 0x00, 0x48, 0x31, 0x91, 0x90, 0x00, 0x00, 0x00, 0x48,
		0x31, 0x91, 0x98, 0x00, 0x00, 0x00, 0x48, 0x31, 0x91, 0xA0,
		0x00, 0x00, 0x00, 0x48, 0x31, 0x91, 0xA8, 0x00, 0x00, 0x00,
		0x48, 0x31, 0x91, 0xB0, 0x00, 0x00, 0x00, 0x48, 0x31, 0x91,
		0xB8, 0x00, 0x00, 0x00, 0x48, 0x31, 0x91, 0xC0, 0x00, 0x00,
		0x00, 0x31, 0x11, 0x48, 0x83, 0xE9, 0x48, 0x4C, 0x8B, 0x41,
		0x40, 0x4D, 0x8B, 0x50, 0x40, 0x48, 0xBA, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x80, 0xFF, 0xFF, 0x4D, 0x8B, 0x48, 0x20, 0x4D,
		0x33, 0xD1, 0x4C, 0x0B, 0xD2, 0x48, 0xBA, 0x2E, 0x48, 0x31,
		0x11, 0x48, 0x31, 0x51, 0x08, 0x48, 0x8B, 0xCA, 0x49, 0x33,
		0x12, 0x41, 0x89, 0x0A, 0x49, 0x8B, 0xCA, 0xFF, 0xE1
	};

	//7600 7601
	static const UCHAR Code_Win7[] =
	{
		0x4E, 0x8D, 0x5C, 0x23, 0xFF, 0x41, 0xBE, 0x01, 0x00, 0x00,
		0x00, 0x41, 0x8D, 0x41, 0xFF, 0x23, 0xF0
	};
	const auto CodeOffset_PoolBigPageTable_win7 = 23;
	const auto CodeOffset_PoolBigPageTableSize_win7 = 7;

	//14393
	static const UCHAR Code_Win10[] =
	{
		0x44, 0x0F, 0xB6, 0xE0, 0x49, 0xC1, 0xE8, 0x20, 0x44, 0x33,
		0xC7, 0x41, 0x8D, 0x51, 0xFF, 0x44, 0x23, 0xC2, 0x4D, 0x85,
		0xD2
	};
	const auto CodeOffset_PoolBigPageTable_win10 = 7;
	const auto CodeOffset_PoolBigPageTableSize_win10 = 17;

	//9200
	static const UCHAR Code_Win8[] =
	{
		0x41, 0x8D, 0x40, 0xFF, 0x23, 0xF8, 0x4D, 0x85, 0xC9
	};
	const auto CodeOffset_PoolBigPageTable_win8 = 7;
	const auto CodeOffset_PoolBigPageTableSize_win8 = 14;

	//9600
	static const UCHAR Code_Win81[] =
	{
		0x41, 0x8D, 0x40, 0xFF, 0x23, 0xD8, 0x4D, 0x85, 0xC9
	};
	const auto CodeOffset_PoolBigPageTable_win81 = 7;
	const auto CodeOffset_PoolBigPageTableSize_win81 = 14;

	struct PatchGuardContextInfo
	{
		ULONG_PTR PgContext;    // An address of PatchGuard context
		ULONG64 XorKey;         // XorKey to decrypt it, or 0 when it has already
								// been decrypted.
	};

	bool IsCmpAppendDllSection(
		__in const ULONG64* AddressToBeChecked,
		__in ULONG64 PossibleXorKey)
	{
		const auto NUMBER_OF_TIMES_TO_COMPARE =
			sizeof(CmpAppendDllSection_PATTERN) / sizeof(ULONG64);
		C_ASSERT(NUMBER_OF_TIMES_TO_COMPARE == 19);

		for (int i = 2; i < NUMBER_OF_TIMES_TO_COMPARE; ++i)
		{
			const auto decryptedContents = AddressToBeChecked[i] ^ PossibleXorKey;
			if (decryptedContents != CmpAppendDllSection_PATTERN[i])
			{
				return false;
			}
		}
		return true;
	}
	bool IsKiDpcDispatch(
		__in const ULONG64* AddressToBeChecked,
		__in ULONG64 PossibleXorKey)
	{
		const auto NUMBER_OF_TIMES_TO_COMPARE =
			sizeof(KiDpcDispatch_PATTERN) / sizeof(ULONG64);
		C_ASSERT(NUMBER_OF_TIMES_TO_COMPARE == 23);

		auto _KiDpcDispatch_PATTERN = (ULONG64*)(KiDpcDispatch_PATTERN);
		for (int i = 2; i < NUMBER_OF_TIMES_TO_COMPARE; ++i)
		{
			const auto decryptedContents = AddressToBeChecked[i] ^ PossibleXorKey;
			if (decryptedContents != _KiDpcDispatch_PATTERN[i])
			{
				return false;
			}
		}
		return true;
	}

	SIZE_T SearchPatchGuardContext(
		__in ULONG_PTR SearchBase,
		__in SIZE_T SearchSize,
		__out PatchGuardContextInfo& Result)
	{
		const auto maxSearchSize =
			SearchSize - sizeof(CmpAppendDllSection_PATTERN);
		for (SIZE_T searchedBytes = 0; searchedBytes < maxSearchSize;
			++searchedBytes)
		{
			const auto addressToBeChecked =
				reinterpret_cast<ULONG64*>(SearchBase + searchedBytes);

			const auto possibleXorKey =
				addressToBeChecked[1] ^ CmpAppendDllSection_PATTERN[1];
			if (IsCmpAppendDllSection(addressToBeChecked, possibleXorKey))
			{
				// A PatchGuard context was found
				Result.PgContext = reinterpret_cast<ULONG_PTR>(addressToBeChecked);
				Result.XorKey = possibleXorKey;
				return searchedBytes + 1;
			}
		}
		return SearchSize;
	}
	SIZE_T SearchPatchGuardContext2(
		__in ULONG_PTR SearchBase,
		__in SIZE_T SearchSize,
		__out PatchGuardContextInfo& Result)
	{
		const auto maxSearchSize =
			SearchSize - sizeof(KiDpcDispatch_PATTERN);
		auto _KiDpcDispatch_PATTERN = (ULONG64*)(KiDpcDispatch_PATTERN);
		for (SIZE_T searchedBytes = 0; searchedBytes < maxSearchSize;
			++searchedBytes)
		{
			const auto addressToBeChecked =
				reinterpret_cast<ULONG64*>(SearchBase + searchedBytes);

			const auto possibleXorKey =
				addressToBeChecked[1] ^ _KiDpcDispatch_PATTERN[1];
			if (IsKiDpcDispatch(addressToBeChecked, possibleXorKey))
			{
				// A PatchGuard context was found
				Result.PgContext = reinterpret_cast<ULONG_PTR>(addressToBeChecked);
				Result.XorKey = possibleXorKey;
				return searchedBytes + 1;
			}
		}
		return SearchSize;
	}
	bool check_pg1(PVOID MapAddress, SIZE_T ScanSize)
	{
		auto StartAddress = reinterpret_cast<ULONG_PTR>(MapAddress);
		for (SIZE_T searchedBytes = 0; searchedBytes < ScanSize; /**/)
		{
			// Search a context
			PatchGuardContextInfo result = {};
			const auto remainingBytes = ScanSize - searchedBytes;
			const auto searchPosition = StartAddress + searchedBytes;
			const auto checkedBytes = SearchPatchGuardContext(
				searchPosition, remainingBytes, result);//这里有很奇怪的问题，那就是为何Win8.1和Win10上搜不到呢？
			searchedBytes += checkedBytes;

			// Check if a context was found
			if (result.PgContext)
			{
				return true;
			}
		}
		return false;
	}
	bool check_pg2(PVOID MapAddress, SIZE_T ScanSize)
	{
		auto StartAddress = reinterpret_cast<ULONG_PTR>(MapAddress);
		for (SIZE_T searchedBytes = 0; searchedBytes < ScanSize; /**/)
		{
			// Search a context
			PatchGuardContextInfo result = {};
			const auto remainingBytes = ScanSize - searchedBytes;
			const auto searchPosition = StartAddress + searchedBytes;
			const auto checkedBytes = SearchPatchGuardContext2(
				searchPosition, remainingBytes, result);//这里有很奇怪的问题，那就是为何Win8.1和Win10上搜不到呢？
			searchedBytes += checkedBytes;

			// Check if a context was found
			if (result.PgContext)
			{
				return true;
			}
		}
		return false;
	}
	bool check_pg3(PVOID MapAddress, SIZE_T ScanSize)
	{
		auto GetRamdomness = [](
			__in void* Addr,
			__in SIZE_T Size)
		{
			const auto p = static_cast<UCHAR*>(Addr);
			std::set<UCHAR> dic;
			for (SIZE_T i = 0; i < Size; ++i)
			{
				dic.insert(p[i]);
			}
			return static_cast<ULONG>(dic.size());
		};
		auto GetNumberOfDistinctiveNumbers = [](
			__in void* Addr,
			__in SIZE_T Size)
		{
			const auto p = static_cast<UCHAR*>(Addr);
			ULONG count = 0;
			for (SIZE_T i = 0; i < Size; ++i)
			{
				if (p[i] == 0xff || p[i] == 0x00)
				{
					count++;
				}
			}
			return count;
		};
		const auto EXAMINATION_BYTES = 100;
		const auto MAXIMUM_DISTINCTIVE_NUMBER = 5;
		const auto MINIMUM_RANDOMNESS = 50;
		const auto MINIMUM_REGION_SIZE = 0x004000;
		const auto MAXIMUM_REGION_SIZE = 0xf00000;

		auto StartAddress = reinterpret_cast<ULONG_PTR>(MapAddress);
		for (SIZE_T searchedBytes = 0; searchedBytes < ScanSize; searchedBytes += PAGE_SIZE)
		{
			// Search a context
			const auto remainingBytes = ScanSize - searchedBytes;
			const auto searchPosition = StartAddress + searchedBytes;
			if (remainingBytes < 100)
			{
				break;
			}
			const auto numberOfDistinctiveNumbers =
				GetNumberOfDistinctiveNumbers(
					PVOID(searchPosition), EXAMINATION_BYTES);
			const auto randomness = GetRamdomness(
				PVOID(searchPosition), EXAMINATION_BYTES);
			if (numberOfDistinctiveNumbers > MAXIMUM_DISTINCTIVE_NUMBER
				|| randomness < MINIMUM_RANDOMNESS)
			{
				continue;
			}
			if (remainingBytes < MINIMUM_REGION_SIZE)
			{
				LOG_DEBUG("%p %x\r\n", searchPosition, remainingBytes);
				continue;
			}
			LOG_DEBUG("find some memory like pg %p\r\n", PVOID(searchPosition));
			return true;
			//return true;
		}
		return false;
	}
	bool scan_pool_mem(PVOID mapAddress, SIZE_T ScanSize)
	{
		if (ScanSize<PAGE_SIZE+1)
		{
			return false;
		}
		LOG_DEBUG("Scan Pool %p size = %x\r\n", mapAddress, ScanSize);
		if (!ddk::mem_util::MmIsExecutableAddress(mapAddress))
		{
			return false;
		}
		PVOID findImage = nullptr;
		RtlPcToFileHeader(mapAddress, &findImage);
		if (findImage)
		{
			return false;
		}
		auto pde = ddk::mem_util::UtilpAddressToPde(mapAddress);
		auto pte = ddk::mem_util::UtilpAddressToPte(mapAddress);
		if ((pde->LargePage && pde->Write)
			|| (!pde->LargePage && pte &&pte->Write))
		{

			if (check_pg1(mapAddress,ScanSize)
				||check_pg2(mapAddress,ScanSize)
				||check_pg3(mapAddress, ScanSize))
			{
				LOG_DEBUG("pg find va=%p\r\n", mapAddress);
				for (auto p = 0; p < ScanSize; p += PAGE_SIZE)
				{
					ddk::mem_util::MmSetAddresssNoExecutable(PVOID((PUCHAR)mapAddress + p));
				}
				KeInvalidateAllCaches();
				return true;
			}
		}
		return false;
	}
	void scan_pool_mem()
	{
		PVOID lpNtMem = nullptr;
		auto st = ddk::util::LoadFileToMem(L"\\SystemRoot\\System32\\ntoskrnl.exe", &lpNtMem);
		if (!NT_SUCCESS(st))
		{
			return;
		}
		if (!lpNtMem)
		{
			return;
		}
		auto exit1 = std::experimental::make_scope_exit([&]() {
			if (lpNtMem)
			{
				ExFreePool(lpNtMem);
			}
		});
		//根据系统版本搜索特征码找到PoolBigPageTable和PoolBigPageTableSize
		auto ntosImageBase = ddk::util::get_ntos_imagebase();
		if (!ntosImageBase)
		{
			return;
		}

		auto psigCode = (PVOID)Code_Win7;
		auto sigCodeSize = sizeof(Code_Win7);
		auto table_offset = CodeOffset_PoolBigPageTable_win7;
		auto size_offset = CodeOffset_PoolBigPageTableSize_win7;


		PPOOL_TRACKER_BIG_PAGES lpkvPoolBigPageTable = nullptr;
		ULONG_PTR*lpkvPoolBigPageTableSize = 0;
		auto pImageDosHeader = (PIMAGE_DOS_HEADER)lpNtMem;
		auto pImageNtHeaders = (PIMAGE_NT_HEADERS)((ULONG_PTR)lpNtMem + pImageDosHeader->e_lfanew);
		for (auto Index = 0; Index < pImageNtHeaders->OptionalHeader.SizeOfImage; Index++)
		{
			if (memcmp((PVOID)((ULONG_PTR)lpNtMem + Index), psigCode, sigCodeSize) == 0)
			{
				LARGE_INTEGER Assist = {};
				Assist.QuadPart = (ULONG_PTR)ntosImageBase + Index - table_offset;
				Assist.LowPart += *(ULONG*)(Assist.QuadPart + 3) + 7;
				lpkvPoolBigPageTable = (PPOOL_TRACKER_BIG_PAGES)*(ULONG_PTR*)Assist.QuadPart;

				Assist.QuadPart = (ULONG_PTR)ntosImageBase + Index - size_offset;
				Assist.LowPart += *(ULONG*)(Assist.QuadPart + 3) + 7;
				lpkvPoolBigPageTableSize = (ULONG_PTR*)Assist.QuadPart;
			}
		}
		if (lpkvPoolBigPageTableSize && lpkvPoolBigPageTable)
		{
			for (auto Index = 0; Index < *lpkvPoolBigPageTableSize; Index++)
			{
				auto PoolTrackerBigPagesItem = lpkvPoolBigPageTable[Index];
				auto StartAddress = (ULONG_PTR)PoolTrackerBigPagesItem.Va;
				auto mapAddress = (PVOID)StartAddress;
				if (StartAddress == 0 || (StartAddress & 0x1) != 0)	//为0或1代表未使用
				{
					continue;
				}
				scan_pool_mem(mapAddress, PoolTrackerBigPagesItem.NumberOfBytes);
			}
		}
	}
	void scan_phyfor_pages()
	{
		auto PhysicalMemoryBlock = std::experimental::make_unique_resource(
			MmGetPhysicalMemoryRanges(), &ExFreePool);
		auto phymem = PhysicalMemoryBlock.get();
		if (!phymem)
		{
			return;
		}
		auto i = 0;
		while (phymem[i].NumberOfBytes.QuadPart != 0)
		{
			PHYSICAL_ADDRESS BaseAddress = PhysicalMemoryBlock[i].BaseAddress;
			LARGE_INTEGER NumberOfBytes = PhysicalMemoryBlock[i].NumberOfBytes;
			while (NumberOfBytes.QuadPart > 0)
			{
				auto MapAddress = MmGetVirtualForPhysical(BaseAddress);
				auto ulAddress = reinterpret_cast<ULONG_PTR>(MapAddress);
				const auto HAL_BASE = 0xFFFFFFFFFFD00000ULL;
				SIZE_T ScanSize = PAGE_SIZE;
				if (MapAddress
					&& ulAddress > (ULONG_PTR)MmSystemRangeStart
					&& ulAddress < HAL_BASE
					&& (ulAddress > 0xfffff70000000000ULL
						|| ulAddress < 0xfffff68000000000ULL))//HAL和页表区域不能乱NX=1
				{
					PVOID ImageBase = nullptr;
					if (ddk::mem_util::MmIsExecutableAddress(MapAddress))
					{
						RtlPcToFileHeader(MapAddress, &ImageBase);
						if (!ImageBase)
						{
							//发现无模块的可执行内存	
							auto pde = ddk::mem_util::UtilpAddressToPde(MapAddress);
							auto pte = ddk::mem_util::UtilpAddressToPte(MapAddress);
							if (pde->LargePage)
							{
								ScanSize = 0x200000ui64;
							}
							if (!pde->LargePage && pte && pte->Write)
							{
								//可执行的分页内存必然是NoImageCode
								ddk::mem_util::MmSetAddresssNoExecutable(MapAddress);
								KeInvalidateAllCaches();
							}

							////if(pde->LargePage && pde->Write)
							////{
							////	LOG_DEBUG("ScanLargePage %p\r\n", MapAddress);
							////	//搜索疑似Patchguard的东西
							////	if (check_pg1(MapAddress, ScanSize)
							////		|| check_pg2(MapAddress, ScanSize)
							////		|| check_pg3(MapAddress, ScanSize))
							////	{
							////		//设置NX
							////		LOG_DEBUG("pg pa=%p va=%p\r\n", PVOID(BaseAddress.QuadPart), MapAddress);
							////		ddk::mem_util::MmSetAddresssNoExecutable(MapAddress);
							////		KeInvalidateAllCaches();
							////	}
							////}
							///*else*/
							////{
							////	//Paged Section
							////	//理论上x64上分页的可以执行内存必然是noimage code
							////	//LOG_DEBUG("noimage code find pa=%p va=%p\r\n", PVOID(BaseAddress.QuadPart), MapAddress);
							////	ddk::mem_util::MmSetAddresssNoExecutable(MapAddress);
							////	KeInvalidateAllCaches();
							////}

						}
					}
				}
				BaseAddress.QuadPart += ScanSize;
				NumberOfBytes.QuadPart -= ScanSize;
			}
			i++;
		}
	}
	void dispg_new()
	{
		IA32_EFER_MSR ia32efer = { __readmsr(0xC0000080) };
		if (ia32efer.fields.NXE)
		{
			LOG_DEBUG("NXE is set ok\r\n");
		}
		else
		{
			LOG_DEBUG("NXE is not set\r\nNewDisPg is not worked\r\n");
			return;
		}
		//PDE 三次搜索可以这样
		ddk::idt_hook::getInstance().MsHookInt(0x1, handle_debugfault_trap, false);
		ddk::idt_hook::getInstance().MsHookInt(0xE, handle_pagefault_trap, true);

		//PDE不精确判断的话，需要用下面的方法：
		//三次搜索方式进行判断非常慢慢慢，上面MsHookInt使用的都是NonPagedPool,所以你懂得
		//ddk::idt_hook::getInstance().hookInt(0x1, (PVOID)debugfault_trap, old_debugfault_handler);
		//ddk::idt_hook::getInstance().hookInt(0xE, (PVOID)pagefault_trap, old_pftrap_handler);

		//测试一下#PF改RIP
		PF_Test();
		RtlZeroMemory(mtf_list, sizeof(mtf_list));
		//开始遍历物理内存
		
		_SystemCr3 = __readcr3();
		LOG_DEBUG("MmSystemRangeStart =%p\r\n", MmSystemRangeStart);

		//////////////////////////////////////////////////////////////////////////
		//遍历Pool内存，凡事符合X 且noImage 且可写Write 且随机数测试符合规则
		//怼死
		scan_pool_mem();
		//////////////////////////////////////////////////////////////////////////
		//下面扫物理内存怼PagedPte,不在Image里直接怼死
		//PTE内存，可写可执行，无模块，怼死
		scan_phyfor_pages();
		//////////////////////////////////////////////////////////////////////////
		/*if (g_pDriverObject)
		{
			g_pDriverObject->DriverUnload = nullptr;
		}*/
	}
};