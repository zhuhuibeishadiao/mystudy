#pragma once
namespace ddk::mem_util
{
	void init_mem_util();
	//////////////////////////////////////////////////////////////////////////
	void *MmMemMem(const void *SearchBase,
		SIZE_T SearchSize,
		const void *Pattern,
		SIZE_T PatternSize);
	NTSTATUS MmSearch(
		IN PUCHAR adresseBase,
		IN PUCHAR adresseMaxMin,
		IN PUCHAR pattern,
		OUT PUCHAR *addressePattern,
		IN SIZE_T longueur);
	NTSTATUS MmGenericPointerSearch(
		OUT PUCHAR *addressePointeur,
		IN PUCHAR adresseBase,
		IN PUCHAR adresseMaxMin,
		IN PUCHAR pattern,
		IN SIZE_T longueur,
		IN LONG offsetTo);
	NTSTATUS MmForceMemCpy(
		OUT void *Destination,
		IN const void *Source,
		IN SIZE_T Length);
	//////////////////////////////////////////////////////////////////////////
	struct WINDOWS_RT_PTE {
		ULONG NoExecute : 1;
		ULONG Present : 1;
		ULONG Unknown1 : 5;
		ULONG Writable : 1;
		ULONG Unknown2 : 4;
		ULONG PageFrameNumber : 20;
	};
	static_assert(sizeof(WINDOWS_RT_PTE) == 4, "Size check");

	struct WINDOWS_AMD64_PTE {
		ULONG64 Present : 1;
		ULONG64 Write : 1;
		ULONG64 Owner : 1;
		ULONG64 WriteThrough : 1;
		ULONG64 CacheDisable : 1;
		ULONG64 Accessed : 1;
		ULONG64 Dirty : 1;
		ULONG64 LargePage : 1;
		ULONG64 Global : 1;
		ULONG64 CopyOnWrite : 1;
		ULONG64 Prototype : 1;
		ULONG64 reserved0 : 1;
		ULONG64 PageFrameNumber : 36;
		ULONG64 reserved1 : 4;
		ULONG64 SoftwareWsIndex : 11;
		ULONG64 NoExecute : 1;
	};
	static_assert(sizeof(WINDOWS_AMD64_PTE) == 8, "Size check");
#if defined(_AMD64_)
	typedef struct _PML4E
	{
		union
		{
			struct
			{
				ULONG64 Present : 1;              // Must be 1, region invalid if 0.
				ULONG64 ReadWrite : 1;            // If 0, writes not allowed.
				ULONG64 UserSupervisor : 1;       // If 0, user-mode accesses not allowed.
				ULONG64 PageWriteThrough : 1;     // Determines the memory type used to access PDPT.
				ULONG64 PageCacheDisable : 1;     // Determines the memory type used to access PDPT.
				ULONG64 Accessed : 1;             // If 0, this entry has not been used for translation.
				ULONG64 Ignored1 : 1;
				ULONG64 PageSize : 1;             // Must be 0 for PML4E.
				ULONG64 Ignored2 : 4;
				ULONG64 PageFrameNumber : 36;     // The page frame number of the PDPT of this PML4E.
				ULONG64 Reserved : 4;
				ULONG64 Ignored3 : 11;
				ULONG64 ExecuteDisable : 1;       // If 1, instruction fetches not allowed.
			};
			ULONG64 Value;
		};
	} PML4E, *PPML4E;
	static_assert(sizeof(PML4E) == sizeof(PVOID), "Size mismatch, only 64-bit supported.");
	typedef struct _PDPTE
	{
		union
		{
			struct
			{
				ULONG64 Present : 1;              // Must be 1, region invalid if 0.
				ULONG64 ReadWrite : 1;            // If 0, writes not allowed.
				ULONG64 UserSupervisor : 1;       // If 0, user-mode accesses not allowed.
				ULONG64 PageWriteThrough : 1;     // Determines the memory type used to access PD.
				ULONG64 PageCacheDisable : 1;     // Determines the memory type used to access PD.
				ULONG64 Accessed : 1;             // If 0, this entry has not been used for translation.
				ULONG64 Ignored1 : 1;
				ULONG64 PageSize : 1;             // If 1, this entry maps a 1GB page.
				ULONG64 Ignored2 : 4;
				ULONG64 PageFrameNumber : 36;     // The page frame number of the PD of this PDPTE.
				ULONG64 Reserved : 4;
				ULONG64 Ignored3 : 11;
				ULONG64 ExecuteDisable : 1;       // If 1, instruction fetches not allowed.
			}page_table;
			struct 
			{
				ULONG64 Present : 1;              // Must be 1, region invalid if 0.
				ULONG64 ReadWrite : 1;            // If 0, writes not allowed.
				ULONG64 UserSupervisor : 1;       // If 0, user-mode accesses not allowed.
				ULONG64 PageWriteThrough : 1;     // Determines the memory type used to access PT.
				ULONG64 PageCacheDisable : 1;     // Determines the memory type used to access PT.
				ULONG64 Accessed : 1;             // If 0, this entry has not been used for translation.
				ULONG64 Dirty : 1;
				ULONG64 PageSize : 1;             // If 1, this entry maps a 1GB page.
				ULONG64 Global : 1;
				ULONG64 Ignored1 : 3;
				ULONG64 PageAccessType : 1;
				ULONG64 Reserved : 17;
				ULONG64 PageFrameNumber : 18; //1GBµÄPFN >>(12+10+10) == PA
				ULONG64 Reserved2 : 4;
				ULONG64 Ignored2 : 7;
				ULONG64 ProtectionKey : 4;
				ULONG64 ExecuteDisable : 1;       // If 1, instruction fetches not allowed.
			}large_page;
			ULONG64 Value;
		};
	} PDPTE, *PPDPTE;
	static_assert(sizeof(PDPTE) == sizeof(PVOID), "Size mismatch, only 64-bit supported.");

	typedef struct _PDE
	{
		union
		{
			struct
			{
				ULONG64 Present : 1;              // Must be 1, region invalid if 0.
				ULONG64 ReadWrite : 1;            // If 0, writes not allowed.
				ULONG64 UserSupervisor : 1;       // If 0, user-mode accesses not allowed.
				ULONG64 PageWriteThrough : 1;     // Determines the memory type used to access PT.
				ULONG64 PageCacheDisable : 1;     // Determines the memory type used to access PT.
				ULONG64 Accessed : 1;             // If 0, this entry has not been used for translation.
				ULONG64 Ignored1 : 1;
				ULONG64 PageSize : 1;             // If 1, this entry maps a 2MB page.
				ULONG64 Ignored2 : 4;
				ULONG64 PageFrameNumber : 36;     // The page frame number of the PT of this PDE.
				ULONG64 Reserved : 4;
				ULONG64 Ignored3 : 11;
				ULONG64 ExecuteDisable : 1;       // If 1, instruction fetches not allowed.
			} page_table;
			struct
			{
				ULONG64 Present : 1;              // Must be 1, region invalid if 0.
				ULONG64 ReadWrite : 1;            // If 0, writes not allowed.
				ULONG64 UserSupervisor : 1;       // If 0, user-mode accesses not allowed.
				ULONG64 PageWriteThrough : 1;     // Determines the memory type used to access PT.
				ULONG64 PageCacheDisable : 1;     // Determines the memory type used to access PT.
				ULONG64 Accessed : 1;             // If 0, this entry has not been used for translation.
				ULONG64 Dirty : 1;
				ULONG64 PageSize : 1;             // If 1, this entry maps a 2MB page.
				ULONG64 Global : 1;
				ULONG64 Ignored1 : 3;
				ULONG64 PageAccessType : 1;
				ULONG64 Ignored2 : 8;
				ULONG64 PageFrameNumber : 27; //2MBµÄPFN >>(12+10) == PA
				ULONG64 Reserved : 4;
				ULONG64 Ignored3 : 7;
				ULONG64 ProtectionKey : 4;
				ULONG64 ExecuteDisable : 1;       // If 1, instruction fetches not allowed.
			} large_page;
			ULONG64 Value;
		};
	} PDE, *PPDE;
	static_assert(sizeof(PDE) == sizeof(PVOID), "Size mismatch, only 64-bit supported.");
	typedef struct _PTE
	{
		union
		{
			struct
			{
				ULONG64 Present : 1;              // Must be 1, region invalid if 0.
				ULONG64 ReadWrite : 1;            // If 0, writes not allowed.
				ULONG64 UserSupervisor : 1;       // If 0, user-mode accesses not allowed.
				ULONG64 PageWriteThrough : 1;     // Determines the memory type used to access the memory.
				ULONG64 PageCacheDisable : 1;     // Determines the memory type used to access the memory.
				ULONG64 Accessed : 1;             // If 0, this entry has not been used for translation.
				ULONG64 Dirty : 1;                // If 0, the memory backing this page has not been written to.
				ULONG64 PageAccessType : 1;       // Determines the memory type used to access the memory.
				ULONG64 Global : 1;                // If 1 and the PGE bit of CR4 is set, translations are global.
				ULONG64 Ignored2 : 3;
				ULONG64 PageFrameNumber : 36;    //4KBµÄPFN >>12 ==PA // The page frame number of the backing physical page.
				ULONG64 Reserved : 4;
				ULONG64 Ignored3 : 7;
				ULONG64 ProtectionKey : 4;         // If the PKE bit of CR4 is set, determines the protection key.
				ULONG64 ExecuteDisable : 1;       // If 1, instruction fetches not allowed.
			};
			ULONG64 Value;
		};
	} PTE, *PPTE;
	static_assert(sizeof(PTE) == sizeof(PVOID), "Size mismatch, only 64-bit supported.");

	typedef struct _VIR_ADDRESS
	{
		union 
		{
			struct 
			{
				ULONG64 PageOffset : 12;
				ULONG64 PT_Index : 9;
				ULONG64 PD_Index : 9;
				ULONG64 PDPT_Index : 9;
				ULONG64 PML4_Index : 9;
				ULONG64 Reserved : 16;
			};
			ULONG64 Value;
		};
	}VIR_ADDRESS,*P_VIR_ADDRESS;
	static_assert(sizeof(VIR_ADDRESS) == sizeof(PVOID), "Size mismatch, only 64-bit supported.");
#endif

#ifdef _AMD64_
	using HARDWARE_PTE = WINDOWS_AMD64_PTE;
#else
	using HARDWARE_PTE = WINDOWS_RT_PTE;
#endif
#ifdef _AMD64_
	HARDWARE_PTE *UtilpAddressToPxe(_In_ const void *Address);

	HARDWARE_PTE *UtilpAddressToPpe(_In_ const void *Address);
#endif

	HARDWARE_PTE *UtilpAddressToPde(_In_ const void *Address);
	HARDWARE_PTE *UtilpAddressToPte(_In_ const void *Address);
	//////////////////////////////////////////////////////////////////////////
	bool MmIsAddressNonCanonical(DWORD64 address);
	bool MmIsAccessibleAddress(const void *Address);
	bool MmIsExecutableAddress(const void *Address);
	bool MmSetAddresssNoExecutable(const void *Address);
	bool MmSetAddresssExecutable(const void *Address);
	//bool MmIsMmBase(const void *Address);
	//////////////////////////////////////////////////////////////////////////
	//HARDWARE_PTE* MiConvertNonPagedToPaged(DWORD64 Cr3, const void*Address);
	//HARDWARE_PTE* UtilpAddressToPte(DWORD64 Cr3, const void *Address);
	//bool MmIsExecutableAddress(DWORD64 cr3, const void *address);
	//bool MmIsAccessibleAddress(DWORD64 cr3, const void *address);
	//////////////////////////////////////////////////////////////////////////
	typedef struct _MM_SYSTEM_MAP
	{
		PMDL pMdl;
		PVOID pMappedAddress;
	}MM_SYSTEM_MAP,*PMM_SYSTEM_MAP;
	typedef struct _MM_SYSTEM_ALLOC
	{
		HANDLE hSection;
		PVOID SectionObject;
		PVOID MapAddress;
	}MM_SYSTEM_ALLOC,*PMM_SYSTEM_ALLOC;

	bool MmMapSystemMemoryWritable(PVOID Address, SIZE_T _MapSize,PMM_SYSTEM_MAP pMap);
	void MmFreeMap(PMM_SYSTEM_MAP pMap);
	//////////////////////////////////////////////////////////////////////////
	bool alloc_mem(PVOID wanna_address, size_t alloc_size, PMM_SYSTEM_ALLOC _allocRet);
	void free_alloc(PMM_SYSTEM_ALLOC _allocMem);
	//////////////////////////////////////////////////////////////////////////
	PVOID MmAllocateCcMemory(SIZE_T _alloc_size);
	void MmFreeCcMemory(PVOID _buffer);
	//////////////////////////////////////////////////////////////////////////
	typedef struct _MM_IMAGE_
	{
		PVOID pImage;
		PMDL pImageMdl;
	}MM_IMAGE,*PMM_IMAGE;
	bool MmAllocateImageMemory(SIZE_T number_of_bytes, PMM_IMAGE pImage);
	void MmFreeImageMemory(PMM_IMAGE pImage);

	typedef struct _MM_LOCKED_MEM
	{
		PVOID OrgBuffer;
		PVOID MappedBuffer;
		PMDL MapMdl;
	}MM_LOCKED_MEM,*PMM_LOCKED_MEM;
	bool MmAllocateLockedMemory(SIZE_T _size, PMM_LOCKED_MEM pLockMem);
	void MmFreeLockedMemory(PMM_LOCKED_MEM pLockMem);

	PVOID MmAllocMemoryForHook(PVOID _target, size_t _size);
	//////////////////////////////////////////////////////////////////////////
	/// VA -> PA
	/// @param va   A virtual address to get its physical address
	/// @return A physical address of \a va, or nullptr
	///
	/// @warning
	/// It cannot be used for a virtual address managed by a prototype PTE.
	ULONG64 UtilPaFromVa(_In_ void *va);

	/// VA -> PFN
	/// @param va   A virtual address to get its physical address
	/// @return A page frame number of \a va, or 0
	///
	/// @warning
	/// It cannot be used for a virtual address managed by a prototype PTE.
	PFN_NUMBER UtilPfnFromVa(_In_ void *va);

	/// PA -> PFN
	/// @param pa   A physical address to get its page frame number
	/// @return A page frame number of \a pa, or 0
	PFN_NUMBER UtilPfnFromPa(_In_ ULONG64 pa);

	/// PA -> VA
	/// @param pa   A physical address to get its virtual address
	/// @return A virtual address \a pa, or 0
	void *UtilVaFromPa(_In_ ULONG64 pa);

	/// PNF -> PA
	/// @param pfn   A page frame number to get its physical address
	/// @return A physical address of \a pfn
	ULONG64 UtilPaFromPfn(_In_ PFN_NUMBER pfn);

	/// PNF -> VA
	/// @param pfn   A page frame number to get its virtual address
	/// @return A virtual address of \a pfn
	void *UtilVaFromPfn(_In_ PFN_NUMBER pfn);

	PVOID MiAllocatePageTable();
	bool MiConvertPdeToPtes(PPDE _pde);
	bool MiMapAddressToMapAddress(PVOID MapAddress, PVOID OrgAddress);
	bool MiMarkAddressUserAccess(PVOID address);

	//////////////////////////////////////////////////////////////////////////
	bool MiAllocateMemorySpecialAddress(PVOID _TargetAddress, SIZE_T _Size);
}