#include "stdafx.h"

namespace ddk::mem_util
{
	void *MmMemMem(const void *SearchBase,
		SIZE_T SearchSize,
		const void *Pattern,
		SIZE_T PatternSize) {
		if (PatternSize > SearchSize) {
			return nullptr;
		}
		auto searchBase = static_cast<const char *>(SearchBase);
		for (size_t i = 0; i <= SearchSize - PatternSize; i++) {
			if (!memcmp(Pattern, &searchBase[i], PatternSize)) {
				return const_cast<char *>(&searchBase[i]);
			}
		}
		return nullptr;
	}
	NTSTATUS MmSearch(
		IN PUCHAR adresseBase,
		IN PUCHAR adresseMaxMin,
		IN PUCHAR pattern,
		OUT PUCHAR *addressePattern,
		IN SIZE_T longueur)
	{
		for (*addressePattern = adresseBase;
			(adresseMaxMin > adresseBase) ? (*addressePattern <= adresseMaxMin) : (*addressePattern >= adresseMaxMin);
			*addressePattern += (adresseMaxMin > adresseBase) ? 1 : -1)
			if (RtlEqualMemory(pattern, *addressePattern, longueur))
				return STATUS_SUCCESS;
		*addressePattern = NULL;
		return STATUS_NOT_FOUND;
	}

	NTSTATUS MmGenericPointerSearch(
		OUT PUCHAR *addressePointeur,
		IN PUCHAR adresseBase,
		IN PUCHAR adresseMaxMin,
		IN PUCHAR pattern,
		IN SIZE_T longueur,
		IN LONG offsetTo)
	{
		NTSTATUS status = MmSearch(adresseBase,
			adresseMaxMin,
			pattern,
			addressePointeur,
			longueur);
		if (NT_SUCCESS(status))
		{
			*addressePointeur += offsetTo;
#ifdef _AMD64_
			*addressePointeur += sizeof(LONG) + *(PLONG)(*addressePointeur);
#else
			*addressePointeur = *(PUCHAR *)(*addressePointeur);
#endif

			if (!*addressePointeur)
				status = STATUS_INVALID_HANDLE;
		}
		return status;
	}
	NTSTATUS MmForceMemCpy(
		OUT void *Destination,
		IN const void *Source,
		IN SIZE_T Length)
	{
		auto mdl = std::experimental::make_unique_resource(
			IoAllocateMdl(Destination, static_cast<ULONG>(Length), FALSE, FALSE,
				nullptr),
			&IoFreeMdl);
		if (!mdl) {
			return STATUS_INSUFFICIENT_RESOURCES;
		}
		MmBuildMdlForNonPagedPool(mdl.get());

#pragma warning(push)
#pragma warning(disable : 28145)
		//
		// Following MmMapLockedPagesSpecifyCache() call causes bug check in case
		// you are using Driver Verifier. The reason is explained as follows:
		//
		// A driver must not try to create more than one system-address-space
		// mapping for an MDL. Additionally, because an MDL that is built by the
		// MmBuildMdlForNonPagedPool routine is already mapped to the system
		// address space, a driver must not try to map this MDL into the system
		// address space again by using the MmMapLockedPagesSpecifyCache routine.
		// -- MSDN
		//
		// This flag modification hacks Driver Verifier's check and prevent leading
		// bug check.
		//
		mdl.get()->MdlFlags &= ~MDL_SOURCE_IS_NONPAGED_POOL;
		mdl.get()->MdlFlags |= MDL_PAGES_LOCKED;
#pragma warning(pop)

		auto writableDest = MmMapLockedPagesSpecifyCache(
			mdl.get(), KernelMode, MmCached, nullptr, FALSE, NormalPagePriority);
		if (!writableDest) {
			return STATUS_INSUFFICIENT_RESOURCES;
		}
		memcpy(writableDest, Source, Length);
		MmUnmapLockedPages(writableDest, mdl.get());
		return STATUS_SUCCESS;
	}
	//////////////////////////////////////////////////////////////////////////
	// Base addresses of page structures. Use !pte to obtain them.
	static auto kUtilpPxeBase = 0xfffff6fb7dbed000ull;
	static auto kUtilpPpeBase = 0xfffff6fb7da00000ull;
	static auto kUtilpPdeBase = 0xfffff6fb40000000ull;
	static auto kUtilpPteBase = 0xfffff68000000000ull;

	// Get the highest 25 bits
	static const auto kUtilpPxiShift = 39ull;

	// Get the highest 34 bits
	static const auto kUtilpPpiShift = 30ull;

	// Get the highest 43 bits
	static const auto kUtilpPdiShift = 21ull;

	// Get the highest 52 bits
	static const auto kUtilpPtiShift = 12ull;

	// Use  9 bits; 0b0000_0000_0000_0000_0000_0000_0001_1111_1111
	static const auto kUtilpPxiMask = 0x1ffull;

	// Use 18 bits; 0b0000_0000_0000_0000_0011_1111_1111_1111_1111
	static const auto kUtilpPpiMask = 0x3ffffull;

	// Use 27 bits; 0b0000_0000_0111_1111_1111_1111_1111_1111_1111
	static const auto kUtilpPdiMask = 0x7ffffffull;

	// Use 36 bits; 0b1111_1111_1111_1111_1111_1111_1111_1111_1111
	static const auto kUtilpPtiMask = 0xfffffffffull;


	ULONG_PTR g_utilp_pxe_base = 0;
	ULONG_PTR g_utilp_ppe_base = 0;
	ULONG_PTR g_utilp_pde_base = 0;
	ULONG_PTR g_utilp_pte_base = 0;

	ULONG_PTR g_utilp_pxi_shift = 0;
	ULONG_PTR g_utilp_ppi_shift = 0;
	ULONG_PTR g_utilp_pdi_shift = 0;
	ULONG_PTR g_utilp_pti_shift = 0;

	ULONG_PTR g_utilp_pxi_mask = 0;
	ULONG_PTR g_utilp_ppi_mask = 0;
	ULONG_PTR g_utilp_pdi_mask = 0;
	ULONG_PTR g_utilp_pti_mask = 0;

	ULONG_PTR PAGE_PA_MASK = (0xFFFFFFFFFULL << PAGE_SHIFT);
	void* g_MmPfnDatabase = nullptr;
	//////////////////////////////////////////////////////////////////////////
	// Return an address of PXE
	HARDWARE_PTE *UtilpAddressToPxe(
		const void *Address) {

		const auto addr = reinterpret_cast<ULONG_PTR>(Address);
		const auto pxe_index = (addr >> g_utilp_pxi_shift) & g_utilp_pxi_mask;
		const auto offset = pxe_index * sizeof(HARDWARE_PTE);
		return reinterpret_cast<HARDWARE_PTE *>(g_utilp_pxe_base + offset);
	}

	// Return an address of PPE
	HARDWARE_PTE *UtilpAddressToPpe(
		const void *Address) {

		const auto addr = reinterpret_cast<ULONG_PTR>(Address);
		const auto ppe_index = (addr >> g_utilp_ppi_shift) & g_utilp_ppi_mask;
		const auto offset = ppe_index * sizeof(HARDWARE_PTE);
		return reinterpret_cast<HARDWARE_PTE *>(g_utilp_ppe_base + offset);
	}

	// Return an address of PDE
	HARDWARE_PTE *UtilpAddressToPde(
		const void *Address) {

		const auto addr = reinterpret_cast<ULONG_PTR>(Address);
		const auto pde_index = (addr >> g_utilp_pdi_shift) & g_utilp_pdi_mask;
		const auto offset = pde_index * sizeof(HARDWARE_PTE);
		return reinterpret_cast<HARDWARE_PTE *>(g_utilp_pde_base + offset);
	}

	// Return an address of PTE
	HARDWARE_PTE *UtilpAddressToPte(
		const void *Address) {

		const auto addr = reinterpret_cast<ULONG_PTR>(Address);
		const auto pte_index = (addr >> g_utilp_pti_shift) & g_utilp_pti_mask;
		const auto offset = pte_index * sizeof(HARDWARE_PTE);
		return reinterpret_cast<HARDWARE_PTE *>(g_utilp_pte_base + offset);
	}
	//////////////////////////////////////////////////////////////////////////
	void init_mem_util()
	{
		static bool init_ok = false;
		if (init_ok)
		{
			return;
		}
#ifdef _AMD64_
		RTL_OSVERSIONINFOW os_version = { sizeof(os_version) };
		auto status = RtlGetVersion(&os_version);
		if (!NT_SUCCESS(status)) {
			LOG_DEBUG("RtlGetVersion failed\r\n");
			return;
		}

		if (os_version.dwMajorVersion < 10 ||
			os_version.dwBuildNumber < 14316)
		{
			g_utilp_pxe_base = kUtilpPxeBase;
			g_utilp_ppe_base = kUtilpPpeBase;
			g_utilp_pxi_shift = kUtilpPxiShift;
			g_utilp_ppi_shift = kUtilpPpiShift;
			g_utilp_pxi_mask = kUtilpPxiMask;
			g_utilp_ppi_mask = kUtilpPpiMask;

			g_utilp_pde_base = kUtilpPdeBase;
			g_utilp_pte_base = kUtilpPteBase;
			g_utilp_pdi_shift = kUtilpPdiShift;
			g_utilp_pti_shift = kUtilpPtiShift;
			g_utilp_pdi_mask = kUtilpPdiMask;
			g_utilp_pti_mask = kUtilpPtiMask;

			g_MmPfnDatabase = reinterpret_cast<void *>(0xfffffa8000000000ULL);

			init_ok = true;
			LOG_DEBUG("mem init ok\r\n");
			return;
		}

		// Get PTE_BASE from MmGetVirtualForPhysical
		const auto p_MmGetVirtualForPhysical = ddk::util::get_proc_address("MmGetVirtualForPhysical");
		if (!p_MmGetVirtualForPhysical) {
			LOG_DEBUG("get MmGetVirtualForPhysical failed\r\n");
			return;
		}

		static const UCHAR kPatternWin10x64[] = {
			0x48, 0x8b, 0x04, 0xd0,  // mov     rax, [rax+rdx*8]
			0x48, 0xc1, 0xe0, 0x19,  // shl     rax, 19h
			0x48, 0xba,              // mov     rdx, ????????`????????  ; PTE_BASE
		};



		auto found = reinterpret_cast<ULONG_PTR>(MmMemMem(p_MmGetVirtualForPhysical, 0x30, kPatternWin10x64,
			sizeof(kPatternWin10x64)));
		if (!found) {
			LOG_DEBUG("can not found signature for PTEBASE\r\n");
			return;
		}


		found += sizeof(kPatternWin10x64);

		const auto pte_base = *reinterpret_cast<ULONG_PTR *>(found);

		LOG_DEBUG("PTEBASE = %p\r\n", PVOID(pte_base));

		const auto index = (pte_base >> kUtilpPxiShift) & kUtilpPxiMask;
		const auto pde_base = pte_base | (index << kUtilpPpiShift);
		const auto ppe_base = pde_base | (index << kUtilpPdiShift);
		const auto pxe_base = ppe_base | (index << kUtilpPtiShift);

		g_utilp_pxe_base = static_cast<ULONG_PTR>(pxe_base);
		g_utilp_ppe_base = static_cast<ULONG_PTR>(ppe_base);
		g_utilp_pde_base = static_cast<ULONG_PTR>(pde_base);
		g_utilp_pte_base = static_cast<ULONG_PTR>(pte_base);

		g_utilp_pxi_shift = kUtilpPxiShift;
		g_utilp_ppi_shift = kUtilpPpiShift;
		g_utilp_pdi_shift = kUtilpPdiShift;
		g_utilp_pti_shift = kUtilpPtiShift;

		g_utilp_pxi_mask = kUtilpPxiMask;
		g_utilp_ppi_mask = kUtilpPpiMask;
		g_utilp_pdi_mask = kUtilpPdiMask;
		g_utilp_pti_mask = kUtilpPtiMask;

		static const UCHAR kPatternWin10x64_pfn[] = {
			0x48, 0x8B, 0xC1,        // mov     rax, rcx
			0x48, 0xC1, 0xE8, 0x0C,  // shr     rax, 0Ch
			0x48, 0x8D, 0x14, 0x40,  // lea     rdx, [rax + rax * 2]
			0x48, 0x03, 0xD2,        // add     rdx, rdx
			0x48, 0xB8,              // mov     rax, 0FFFFFA8000000008h
		};

		auto found2 = reinterpret_cast<ULONG_PTR>(MmMemMem(p_MmGetVirtualForPhysical, 0x20, kPatternWin10x64_pfn,
			sizeof(kPatternWin10x64_pfn)));
		if (!found2)
		{
			LOG_DEBUG("can not found sig for pfnDataBase\r\n");
			return;
		}
		found2 += sizeof(kPatternWin10x64_pfn);
		g_MmPfnDatabase = *reinterpret_cast<void **>(found2);
		//卧槽，Win10还有坑洞，8字节对其
		g_MmPfnDatabase = PAGE_ALIGN(g_MmPfnDatabase);
		LOG_DEBUG("PfnBase = %p\r\n", PVOID(g_MmPfnDatabase));
#endif
		init_ok = true;
		LOG_DEBUG("mem init ok\r\n");
	}
	//////////////////////////////////////////////////////////////////////////
	bool MmIsAddressNonCanonical(DWORD64 address)
	{
		//48 位-63 位和47 位都是一个值的地址就是规范地址
		//(unsigned __int64)(a1 >> 47) >= 0xFFFFFFFFFFFFFFFFui64 || a1 >> 47 == 0
		if ((address >> 47) < 0xFFFFFFFFFFFFFFFFui64 && (address >> 47) != 0)
		{
			//X64地址规则
			return true;
		}
		return false;
	}
	// dt nt!_MMPFN
	struct MmPfnV6 {
		union {
			ULONG_PTR ws_index;
		} u1;
		ULONG_PTR u2;
		ULONG_PTR pte_address;
		ULONG_PTR unknown[3];
	};
	static_assert(sizeof(MmPfnV6) == sizeof(void *) * 6, "Size check");

	struct MmPfnV10 {
		union {
			ULONG_PTR ws_index;
		} u1;
		ULONG_PTR pte_address;
		ULONG_PTR unknown[4];
	};
	static_assert(sizeof(MmPfnV10) == sizeof(void *) * 6, "Size check");

	template <typename T>
	constexpr bool UtilIsInBounds(_In_ const T &value, _In_ const T &min,
		_In_ const T &max) {
		return (min <= value) && (value <= max);
	}

	bool MmIsExecutableAddress(const void *Address)
	{
		if (!MmIsAccessibleAddress(Address)) {
			return false;
		}
		const auto pde = UtilpAddressToPde(Address);
		const auto pte = UtilpAddressToPte(Address);
		if (pde->NoExecute || (!pde->LargePage && (!pte || pte->NoExecute))) {
			return false;
		}
		return true;
	}
	bool MmSetAddresssNoExecutable(const void *Address)
	{
		auto pde = UtilpAddressToPde(Address);
		auto pte = UtilpAddressToPte(Address);
		if (pde->LargePage)
		{
			pde->NoExecute = 1;
			return true;
		}
		if (pte)
		{
			pte->NoExecute = 1;
			return true;
		}
		return false;
	}
	/*bool MmIsMmBase(const void *Address)
	{
		return false;
	}*/
	bool MmSetAddresssExecutable(const void *Address)
	{
		//if (!MmIsAccessibleAddress(Address)) {
		//	LOG_DEBUG("Failed\r\n");
		//	return false;
		//}
		const auto pde = UtilpAddressToPde(Address);
		const auto pte = UtilpAddressToPte(Address);
		if (pde->LargePage)
		{
			pde->NoExecute = 0;
			return true;
		}
		if (pte)
		{
			pte->NoExecute = 0;
			return true;
		}
		return false;
	}
	PVOID  MmpPa2Va(ULONGLONG _VA)
	{
		PHYSICAL_ADDRESS PA = {};
		PA.QuadPart = _VA&PAGE_PA_MASK;
		auto ret = MmGetVirtualForPhysical(PA);
		return ret;
	}
	HARDWARE_PTE *UtilpAddressToPxe(DWORD64 Cr3, const void *Address)
	{
		VIR_ADDRESS va = {};
		va.Value = (ULONG64)Address;
		auto pTable = MmpPa2Va(Cr3);
		if (pTable)
		{
			return (HARDWARE_PTE *)((PUCHAR)pTable + va.PML4_Index * sizeof(ULONGLONG));
		}
		return nullptr;
	}
	HARDWARE_PTE *UtilpAddressToPpe(DWORD64 Cr3, const void *Address)
	{
		VIR_ADDRESS va = {};
		va.Value = (ULONG64)Address;
		auto pPxe = UtilpAddressToPxe(Cr3, Address);
		if (!pPxe)
		{
			return nullptr;
		}
		auto pTable = MmpPa2Va(pPxe->PageFrameNumber << PAGE_SHIFT);
		if (pTable)
		{
			return (HARDWARE_PTE *)((PUCHAR)pTable + va.PDPT_Index * sizeof(ULONGLONG));
		}
		return nullptr;
	}
	HARDWARE_PTE* UtilpAddressToPde(DWORD64 Cr3, const void *Address)
	{
		VIR_ADDRESS va = {};
		va.Value = (ULONG64)Address;
		auto pPpe = UtilpAddressToPpe(Cr3, Address);
		if (!pPpe)
		{
			return nullptr;
		}
		auto pTable = MmpPa2Va(pPpe->PageFrameNumber << PAGE_SHIFT);
		if (pTable)
		{
			return (HARDWARE_PTE *)((PUCHAR)pTable + va.PD_Index * sizeof(ULONGLONG));
		}
		return nullptr;
	}
	HARDWARE_PTE* UtilpAddressToPte(DWORD64 Cr3, const void *Address)
	{
		VIR_ADDRESS va = {};
		va.Value = (ULONG64)Address;
		auto pPde = UtilpAddressToPde(Cr3, Address);
		if (!pPde)
		{
			return nullptr;
		}
		auto pTable = MmpPa2Va(pPde->PageFrameNumber << PAGE_SHIFT);
		if (pTable)
		{
			return (HARDWARE_PTE *)((PUCHAR)pTable + va.PT_Index * sizeof(ULONGLONG));
		}
		return nullptr;
	}
	bool MmIsAccessibleAddress(DWORD64 cr3, const void *address)
	{
		auto pxe = UtilpAddressToPxe(cr3, address);
		if (!pxe || !pxe->Present)
		{
			return false;
		}
		auto ppe = UtilpAddressToPpe(cr3, address);
		if (!ppe || !ppe->Present)
		{
			return false;
		}
		auto pde = UtilpAddressToPde(cr3, address);
		if (!pde || !pde->Present)
		{
			return false;
		}
		const auto pte = UtilpAddressToPte(cr3, address);
		if ((!pde->LargePage && (!pte || !pte->Present)))
		{
			return false;
		}
		return true;
	}
	bool MmIsExecutableAddress(DWORD64 cr3, const void *address)
	{
		if (!MmIsAccessibleAddress(cr3, address))
		{
			return false;
		}
		auto pde = UtilpAddressToPde(cr3, address);
		auto pte = UtilpAddressToPte(cr3, address);
		if (pde->NoExecute || (!pde->LargePage && (!pte || pte->NoExecute))) {
			return false;
		}
		return true;
	}
	/*HARDWARE_PTE* MiConvertNonPagedToPaged(DWORD64 Cr3,const void*Address)
	{
		auto OldPde = UtilpAddressToPde(Cr3,Address);
		if (!OldPde->LargePage)
		{
			return UtilpAddressToPte(Cr3, Address);
		}
		auto pNewPde = MmAllocateMemory(PAGE_SIZE);
		if (pNewPde)
		{
			RtlZeroMemory(pNewPde, PAGE_SIZE);
			auto NewPdePa = MmGetPhysicalAddress(pNewPde);
			auto NewPdePfn = NewPdePa.QuadPart >> PAGE_SHIFT;
			for (ULONG i = 0; i < PAGE_SIZE / sizeof(ULONGLONG); i++)
			{
				auto Pte = (HARDWARE_PTE *)((PUCHAR)pNewPde + i * sizeof(ULONGLONG));
				Pte->Accessed = OldPde->Accessed;
				Pte->Write = 1;
				Pte->Global = 1;
				Pte->Present = 1;
				Pte->Dirty = OldPde->Dirty;
				Pte->PageFrameNumber = OldPde->PageFrameNumber + i;
			}
			_disable();
			OldPde->PageFrameNumber = NewPdePfn;
			OldPde->LargePage = 0;
			_enable();
			VIR_ADDRESS va = {};
			va.Value = (ULONG64)Address;
			auto pte = (HARDWARE_PTE *)((PUCHAR)pNewPde + va.PT_Index * sizeof(ULONGLONG));
			return pte;
		}
		return nullptr;
	}*/
	bool MmIsAccessibleAddress(const void *Address)
	{
		const auto pxe = UtilpAddressToPxe(Address);
		if (!pxe->Present)
		{
			return false;
		}
		const auto ppe = UtilpAddressToPpe(Address);
		if (!ppe->Present)
		{
			return false;
		}
		const auto pde = UtilpAddressToPde(Address);
		if (!pde->Present)
		{
			return false;
		}
		const auto pte = UtilpAddressToPte(Address);
		//PTE
		if ((!pde->LargePage && (!pte || !pte->Present)))
		{
			return false;
		}
		return true;
	}

	bool UtilpIsCanonicalFormAddress(void *address) {
		if (!IsX64()) {
			return true;
		}
		return !UtilIsInBounds(0x0000800000000000ull, 0xffff7fffffffffffull,
			reinterpret_cast<ULONG64>(address));
	}

	bool UtilIsNonPageableAddress(void *address)
	{
		void *pfn_database = g_MmPfnDatabase;
		bool is_v6_kernel = !ddk::util::is_window_10();
		if (!UtilpIsCanonicalFormAddress(address)) {
			return false;
		}

		if (IsX64()) {
			const auto pxe = UtilpAddressToPxe(address);
			const auto ppe = UtilpAddressToPpe(address);
			if (!pxe->Present || !ppe->Present) {
				return false;
			}
		}

		const auto pde = UtilpAddressToPde(address);
		const auto pte = UtilpAddressToPte(address);
		if (!pde->Present) {
			return false;
		}
		if (pde->LargePage) {
			return true;  // A large page is always memory resident
		}
		if (!pte || !pte->Present) {
			return false;
		}

		if (is_v6_kernel) {
			if (reinterpret_cast<MmPfnV6 *>(pfn_database)[pte->PageFrameNumber]
				.u1.ws_index) {
				return false;
			}
		}
		else {
			if (reinterpret_cast<MmPfnV10 *>(pfn_database)[pte->PageFrameNumber]
				.u1.ws_index) {
				return false;
			}
		}

		return true;
	}

	bool MmMapSystemMemoryWritable(PVOID Address, SIZE_T _MapSize, PMM_SYSTEM_MAP pMap)
	{
		auto pMdl = IoAllocateMdl(Address, static_cast<ULONG>(_MapSize), FALSE, FALSE,
			nullptr);
		if (!pMdl)
		{
			return false;
		}
		auto exit_mdl = std::experimental::make_scope_exit([&]() {IoFreeMdl(pMdl); });
		MmBuildMdlForNonPagedPool(pMdl);
		__try
		{
			MmProbeAndLockPages(pMdl, KernelMode, IoModifyAccess);
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
			return false;
		}
		exit_mdl.release();
		auto pMapAddr = (PVOID)MmMapLockedPages(pMdl, KernelMode);
		if (pMapAddr == NULL)
		{
			return false;
		}
		pMap->pMappedAddress = pMapAddr;
		pMap->pMdl = pMdl;
		return true;
	}
	void MmFreeMap(PMM_SYSTEM_MAP pMap)
	{
		MmUnmapLockedPages(pMap->pMappedAddress, pMap->pMdl);
		IoFreeMdl(pMap->pMdl);
	}
	bool alloc_mem(PVOID wanna_address, size_t alloc_size, PMM_SYSTEM_ALLOC _allocRet)
	{
		//怕不是要Cr3

		//wanna_address临近地址分配内存
		//首先找到ImageBase与ImageSize
		//然后尝试 ImageBase+ImageSize+PAGE_SIZE & MASK
		//失败再+PAGE_SIZE
		//MASK 0xFFFFFFFFFFFFF000ULL
		//alloc_address = PVOID(0xFFFFF80100000000ULL);
		PVOID alloc_address = nullptr;
		PVOID imageBase = nullptr;
		RtlPcToFileHeader(wanna_address, &imageBase);
		if (!imageBase)
		{
			//临近内存？？
			alloc_address = PVOID((PUCHAR)wanna_address + PAGE_SIZE);
		}
		else
		{
			alloc_address = PVOID((PUCHAR)imageBase + ddk::util::LdrGetImageSize(imageBase) + PAGE_SIZE);
		}

		auto MaxBase = (DWORD64)alloc_address + 0x0FFFFFFFFULL;
		OBJECT_ATTRIBUTES oa = {};
		HANDLE hSection;
		LARGE_INTEGER SectionSize = {};
		PVOID SectionObject = nullptr;
		SectionSize.QuadPart = PAGE_SIZE;
		InitializeObjectAttributes(&oa, nullptr, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, nullptr, nullptr);

		auto st1 = ZwCreateSection(&hSection, SECTION_ALL_ACCESS, &oa, &SectionSize, PAGE_EXECUTE_READWRITE, SEC_COMMIT, nullptr);
		if (!NT_SUCCESS(st1))
		{
			return false;
		}
		st1 = ObReferenceObjectByHandle(
			hSection,
			SECTION_MAP_READ | SECTION_MAP_WRITE,
			nullptr,
			KernelMode,
			&SectionObject,
			nullptr);
		if (!NT_SUCCESS(st1))
		{
			ZwClose(hSection);
			return false;
		}

		while ((DWORD64)alloc_address < MaxBase)
		{
			alloc_address = PVOID((ULONG_PTR)alloc_address & 0xFFFFFFFFFFFFF000ULL);
			PVOID MapAddress = alloc_address;
			SIZE_T MapSize = PAGE_SIZE;
			LOG_DEBUG("try alloc %p\r\n", alloc_address);
			auto ns = MmMapViewInSystemSpace(SectionObject, &MapAddress, &MapSize);
			if (NT_SUCCESS(ns))
			{
				_allocRet->hSection = hSection;
				_allocRet->SectionObject = SectionObject;
				_allocRet->MapAddress = MapAddress;
				return true;
			}
			alloc_address = PVOID((PUCHAR)alloc_address + PAGE_SIZE);
		}
		ObDereferenceObject(SectionObject);
		ZwClose(hSection);
		return false;
	}
	void free_alloc(PMM_SYSTEM_ALLOC _allocMem)
	{
		MmUnmapViewInSystemSpace(_allocMem->MapAddress);
		ObDereferenceObject(_allocMem->SectionObject);
		ZwClose(_allocMem->hSection);
	}
	PVOID MmAllocateCcMemory(SIZE_T _alloc_size)
	{
		PHYSICAL_ADDRESS highest_acceptable_address = {};
		highest_acceptable_address.QuadPart = -1;
		return MmAllocateContiguousMemory(_alloc_size,
			highest_acceptable_address);
	}
	void MmFreeCcMemory(PVOID _buffer)
	{
		if (_buffer)
			MmFreeContiguousMemory(_buffer);
	}
	//////////////////////////////////////////////////////////////////////////
	bool MmAllocateImageMemory(SIZE_T number_of_bytes, PMM_IMAGE pImage)
	{
		PHYSICAL_ADDRESS start = { 0 }, end = { 0 };
		end.QuadPart = MAXULONG64;
		PVOID ImageBase = nullptr;
		auto _ImageMdl = MmAllocatePagesForMdl(start, end, start, number_of_bytes);
		if (_ImageMdl)
		{
			LOG_DEBUG("make mdl ok\r\n");
			__try
			{
				//MmMapLockedPagesSpecifyCache(_ImageMdl, KernelMode, MmCached, NULL, FALSE, HighPagePriority);
				ImageBase = MmGetSystemAddressForMdlSafe(_ImageMdl, NormalPagePriority);
				if (ImageBase)
				{
					auto nss = MmProtectMdlSystemAddress(_ImageMdl, PAGE_EXECUTE_READWRITE);
					LOG_DEBUG("addr = %p %x\r\n", ImageBase, nss);
					if (nss == STATUS_SUCCESS)
					{
						RtlZeroMemory(ImageBase, number_of_bytes);
						pImage->pImage = ImageBase;
						pImage->pImageMdl = _ImageMdl;
						return true;
					}
				}
			}
			__except (1)
			{
				LOG_DEBUG("What mdl fucked\r\n");
				if (ImageBase)
				{
					MmFreePagesFromMdl(_ImageMdl);
				}
				ExFreePool(_ImageMdl);
				return false;
			}
		}
		if (ImageBase)
		{
			MmFreePagesFromMdl(_ImageMdl);
		}
		if (_ImageMdl)
		{
			ExFreePool(_ImageMdl);
		}
		return false;
	}
	void MmFreeImageMemory(PMM_IMAGE pImage)
	{
		MmFreePagesFromMdl(pImage->pImageMdl);
		ExFreePool(pImage->pImageMdl);
	}
	//////////////////////////////////////////////////////////////////////////
	// VA -> PA
	_Use_decl_annotations_ ULONG64 UtilPaFromVa(void *va) {
		const auto pa = MmGetPhysicalAddress(va);
		return pa.QuadPart;
	}

	// VA -> PFN
	_Use_decl_annotations_ PFN_NUMBER UtilPfnFromVa(void *va) {
		return UtilPfnFromPa(UtilPaFromVa(va));
	}

	// PA -> PFN
	_Use_decl_annotations_ PFN_NUMBER UtilPfnFromPa(ULONG64 pa) {
		return static_cast<PFN_NUMBER>(pa >> PAGE_SHIFT);
	}

	// PA -> VA
	_Use_decl_annotations_ void *UtilVaFromPa(ULONG64 pa) {
		PHYSICAL_ADDRESS pa2 = {};
		pa2.QuadPart = pa;
		return MmGetVirtualForPhysical(pa2);
	}

	// PNF -> PA
	_Use_decl_annotations_ ULONG64 UtilPaFromPfn(PFN_NUMBER pfn) {
		return pfn << PAGE_SHIFT;
	}

	// PFN -> VA
	_Use_decl_annotations_ void *UtilVaFromPfn(PFN_NUMBER pfn) {
		return UtilVaFromPa(UtilPaFromPfn(pfn));
	}
	//////////////////////////////////////////////////////////////////////////
	bool MmAllocateLockedMemory(SIZE_T _size, PMM_LOCKED_MEM pLockMem)
	{
		PVOID Buffer = ExAllocatePool(NonPagedPool, _size);
		if (Buffer)
		{
			RtlZeroMemory(Buffer, PAGE_SIZE);
			// allocate memory descriptor
			PMDL Mdl = IoAllocateMdl(Buffer, (ULONG)_size, FALSE, FALSE, NULL);
			if (Mdl)
			{
				__try
				{
					// lock allocated pages
					MmProbeAndLockPages(Mdl, KernelMode, IoWriteAccess);
				}
				__except (EXCEPTION_EXECUTE_HANDLER)
				{
					LOG_DEBUG(__FUNCTION__"(): MmProbeAndLockPages() EXCEPTION\r\n");

					IoFreeMdl(Mdl);
					ExFreePool(Buffer);
					return false;
				}

				// map allocated pages into the kernel space
				PVOID MappedBuffer = MmMapLockedPagesSpecifyCache(
					Mdl,
					KernelMode,
					MmCached,
					NULL,
					FALSE,
					NormalPagePriority
				);
				if (MappedBuffer)
				{
					pLockMem->MapMdl = Mdl;
					pLockMem->MappedBuffer = MappedBuffer;
					pLockMem->OrgBuffer = Buffer;
					return true;
				}
				else
				{
					LOG_DEBUG(__FUNCTION__"(): MmMapLockedPagesSpecifyCache() fails\r\n");
				}

				MmUnlockPages(Mdl);
				IoFreeMdl(Mdl);
			}
			else
			{
				LOG_DEBUG(__FUNCTION__"(): IoAllocateMdl() fails\r\n");
			}

			ExFreePool(Buffer);
		}
		else
		{
			LOG_DEBUG(__FUNCTION__"(): M_ALLOC() fails\r\n");
		}
		return false;
	}
	void MmFreeLockedMemory(PMM_LOCKED_MEM pLockMem)
	{
		MmUnlockPages(pLockMem->MapMdl);
		IoFreeMdl(pLockMem->MapMdl);
		ExFreePool(pLockMem->OrgBuffer);
	}
	PVOID MiAllocatePageTable()
	{
		MM_LOCKED_MEM mm = {};
		if (MmAllocateLockedMemory(PAGE_SIZE,&mm))
		{
			return mm.MappedBuffer;
		}
		return nullptr;
	}
	bool MiConvertPdeToPtes(PPDE _pde)
	{
		KIRQL oldIrql;
		bool bRet = false;
		if (!_pde->page_table.PageSize)
		{
			LOG_DEBUG("NOT LARGE PDE\r\n");
			return bRet;
		}
		LOG_DEBUG("pfn = %x\r\n", _pde->large_page.PageFrameNumber * 512);
		auto PA = _pde->large_page.PageFrameNumber * 512 * PAGE_SIZE;
		auto pfn = UtilPfnFromPa(PA);
		LOG_DEBUG("pfn2 = %x\r\n", pfn);
		PVOID oldVa = UtilVaFromPa(PA);
		if (!oldVa)
		{
			return bRet;
		}
		LOG_DEBUG("StartVA = %p\r\n", oldVa);
		PVOID newPde = MiAllocatePageTable();
		if (!newPde)
		{
			return bRet;
		}
		LOG_DEBUG("NewTalbe=%p\r\n", newPde);
		auto newPdePfn = UtilPfnFromVa(newPde);
		ddk::cpu_lock _lock;
		_lock.lock();
		oldIrql = KeRaiseIrqlToDpcLevel();
		for (auto i = 0ul; i < 512; i++)
		{
			auto Pte = reinterpret_cast<PPTE>((PUCHAR)newPde + i * sizeof(ULONG64));
			Pte->Value = 0;
			Pte->Present = _pde->large_page.Present;
			Pte->ReadWrite = _pde->large_page.ReadWrite;
			Pte->UserSupervisor = _pde->large_page.UserSupervisor;
			Pte->PageWriteThrough = _pde->large_page.PageWriteThrough;
			Pte->PageCacheDisable = _pde->large_page.PageCacheDisable;
			Pte->Accessed = _pde->large_page.Accessed;
			Pte->Dirty = _pde->large_page.Dirty;
			Pte->PageAccessType = _pde->large_page.PageAccessType;
			Pte->Global = _pde->large_page.Global;
			Pte->ExecuteDisable = _pde->large_page.ExecuteDisable;
			Pte->PageFrameNumber = pfn + i;
			//设置PTE,问题为毛这样子呢？
			auto _Va = PVOID((PUCHAR)oldVa + i*PAGE_SIZE);
			auto _pte = (PPTE)UtilpAddressToPte(_Va);
			{
				_pte->Value = Pte->Value;
			}
		}
		_pde->page_table.PageSize = 0;
		_pde->page_table.PageFrameNumber = newPdePfn;
		_pde->page_table.Ignored1 = 0;
		_pde->page_table.Ignored2 = 0;
		_pde->page_table.Ignored3 = 0;
		_pde->page_table.Reserved = 0;
		KeLowerIrql(oldIrql);
		_lock.unlock();
		KeInvalidateAllCaches();
		bRet = true;
		return bRet;
	}
	//每次只XX一个页
	bool MiMapAddressToMapAddress(PVOID MapAddress, PVOID OrgAddress)
	{
		//首先取MapAddresss的PXE
		//PXE不存在，返回false;
		LOG_DEBUG("%p\r\n", MapAddress);
		VIR_ADDRESS va = {};
	//	auto _cr3 = __readcr3();
		auto _pfn = UtilPfnFromVa(OrgAddress);
		auto _pxe = (PPML4E)UtilpAddressToPxe(MapAddress);
		auto _ppe = UtilpAddressToPpe(MapAddress);
		auto _pde = UtilpAddressToPde(MapAddress);
		auto _pte = (PPTE)UtilpAddressToPte(MapAddress);
		va.Value = (ULONG64)MapAddress;
		if (!_pxe->Present)
		{
			LOG_DEBUG("PXE is not here\r\n");
			return false;
		}
		if (MmIsAccessibleAddress(MapAddress))
		{
			LOG_DEBUG("MapAddress is ok\r\n");
			return false;
		}
		//取PPE，PPE不存在则Create一个
		if (!_ppe->Present)
		{
			if (_ppe->PageFrameNumber)
			{
				LOG_DEBUG("PPE has PFN\r\n");
			}
			//创建PPE
			auto new_ppe_table = MiAllocatePageTable();
			if (!new_ppe_table)
			{
				LOG_DEBUG("alloce table ppe failed\r\n");
				return false;
			}
			auto new_ppe_pfn = UtilPfnFromVa(new_ppe_table);
			auto new_ppe = (PPDPTE)(_ppe);
			new_ppe->Value = _pxe->Value;
			new_ppe->page_table.PageFrameNumber = new_ppe_pfn;
			KeInvalidateAllCaches();
			_pde = (HARDWARE_PTE *)((PUCHAR)new_ppe_table + va.PD_Index * sizeof(ULONG64));
		}
		
		if (!_pde->Present)
		{
			if (_pde->PageFrameNumber)
			{
				LOG_DEBUG("PDE has PFN\r\n");
			}
			//创建PDE
			auto new_pde_table = MiAllocatePageTable();
			if (!new_pde_table)
			{
				LOG_DEBUG("alloc pde table failed\r\n");
				return false;
			}
			auto new_pde_pfn = UtilPfnFromVa(new_pde_table);
			auto new_pde = (PPDE)(_pde);
			new_pde->Value = _pxe->Value;
			new_pde->page_table.PageFrameNumber = new_pde_pfn;
			KeInvalidateAllCaches();
			_pte = (PPTE)((PUCHAR)new_pde_table + va.PT_Index * sizeof(ULONG64));
		}
		
		_pte->Value = _pxe->Value;
		_pte->PageFrameNumber = _pfn;
		if (va.Value>(ULONG64)MmSystemRangeStart)
		{
			_pte->Global = 1;//还需要这个，不然有些进程里没有映射
		}
		KeInvalidateAllCaches();
		return true;
	}
	PVOID MiCloneCr3(DWORD64 _TargetCr3)
	{
		//CloneCr3
		if (__readcr3()==_TargetCr3)
		{
			//clone的就是当前的CR3,对不起无法clone
			return nullptr;
		}
		return nullptr;
	}
	bool MiMarkAddressUserAccess(PVOID address)
	{
		if (!MmIsAccessibleAddress(address))
		{
			return false;
		}
		auto _pxe = (PPML4E)UtilpAddressToPxe(address);
		auto _ppe = (PPDPTE)UtilpAddressToPpe(address);
		auto _pde = (PPDE)UtilpAddressToPde(address);
		auto _pte = (PPTE)UtilpAddressToPte(address);
		_pxe->UserSupervisor = 1;
		_ppe->page_table.UserSupervisor = 1;
		if (_ppe->page_table.PageSize)
		{
			return true;
		}
		_pde->page_table.UserSupervisor = 1;
		if (_pde->page_table.PageSize)
		{
			return true;
		}
		_pte->UserSupervisor = 1;
		return true;
	}
	bool MiAllocateMemorySpecialAddress(PVOID _TargetAddress, SIZE_T _Size)
	{
		auto _RealAllocateSize = ddk::util::AlignSize(_Size, PAGE_SIZE);
		for (auto i=0ull;i<_RealAllocateSize;i+=PAGE_SIZE)
		{
			auto pNewVa = reinterpret_cast<PVOID>((PUCHAR)_TargetAddress + i);
			if (MmIsAccessibleAddress(pNewVa))
			{
				return false;
			}
		}
		MM_LOCKED_MEM mm = {};
		if (!MmAllocateLockedMemory(_RealAllocateSize, &mm))
		{
			return false;
		}
		for (auto i=0ull;i<_RealAllocateSize;i+=PAGE_SIZE)
		{
			auto pNewMapVa = reinterpret_cast<PVOID>((PUCHAR)_TargetAddress + i);
			auto pNewOrgVa = reinterpret_cast<PVOID>((PUCHAR)mm.MappedBuffer + i);
			if (!MiMapAddressToMapAddress(pNewMapVa,pNewOrgVa))
			{
				if (i==0)
				{
					//只有第一次映射才能释放
					MmFreeLockedMemory(&mm);
				}
				return false;
			}
		}
		return true;
	}
	PVOID MmAllocMemoryForHook(PVOID _target, size_t _size)
	{
		PVOID imageBase = nullptr;
		PVOID alloc_address = nullptr;
		RtlPcToFileHeader(_target, &imageBase);
		if (!imageBase)
		{
			//临近内存？？
			alloc_address = PVOID((PUCHAR)_target + PAGE_SIZE);
		}
		else
		{
			alloc_address = PVOID((PUCHAR)imageBase + ddk::util::AlignSize(ddk::util::LdrGetImageSize(imageBase), PAGE_SIZE) + PAGE_SIZE);
		}

		auto MaxBase = (DWORD64)alloc_address + 0x0FFFFFFFFULL;
		while ((DWORD64)alloc_address < MaxBase)
		{
			alloc_address = PVOID((ULONG_PTR)alloc_address & 0xFFFFFFFFFFFFF000ULL);
			if (MiAllocateMemorySpecialAddress(alloc_address,_size))
			{
				return alloc_address;
			}
			alloc_address = PVOID((PUCHAR)alloc_address + PAGE_SIZE);
		}
		return nullptr;
	}
};