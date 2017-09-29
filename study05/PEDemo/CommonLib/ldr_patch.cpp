
#include "stdafx.h"

namespace usr::ldr
{
	bool b_hook = false;
	long _hookRef = 0;
	std::atomic_flag _ldr_lock = ATOMIC_FLAG_INIT;
	using pLdrSetDllManifestProber = VOID(WINAPI *)(PVOID a1, PVOID a2, PVOID a3);
	void ldr_lock() {
		while (_ldr_lock.test_and_set()) {
			std::this_thread::sleep_for(std::chrono::microseconds(1));
		}
	}
	void ldr_unlock() {
		_ldr_lock.clear();
	}
	//////////////////////////////////////////////////////////////////////////
	using T_RtlInitUnicodeString = decltype(&ntdll::RtlInitUnicodeString);
	using T_NtUnmapViewOfSection = decltype(&ntdll::NtUnmapViewOfSection);
	using T_NtOpenFile = decltype(&ntdll::NtOpenFile);
	using T_NtOpenDirectoryObject = decltype(&ntdll::NtOpenDirectoryObject);
	using T_NtOpenSection = decltype(&ntdll::NtOpenSection);
	using T_NtMapViewOfSection = decltype(&ntdll::NtMapViewOfSection);
	using T_RtlNtStatusToDosError = decltype(&ntdll::RtlNtStatusToDosError);
	using T_NtClose = decltype(&ntdll::NtClose);
	using T_NtCreateFile = decltype(&ntdll::NtCreateFile);
	using T_NtCreateSection = decltype(&ntdll::NtCreateSection);
	using T_NtQuerySection = decltype(&ntdll::NtQuerySection);
	using T_LdrLoadDll = decltype(&ntdll::LdrLoadDll);
	using T_RtlCompareUnicodeString = decltype(&ntdll::RtlCompareUnicodeString);
	using T_RtlPushFrame = decltype(&ntdll::RtlPushFrame);
	using T_RtlGetFrame = decltype(&ntdll::RtlGetFrame);
	using T_RtlPopFrame = decltype(&ntdll::RtlPopFrame);
	using T_NtQueryAttributesFile = decltype(&ntdll::NtQueryAttributesFile);
	using T_RtlDosPathNameToNtPathName_U = decltype(&ntdll::RtlDosPathNameToNtPathName_U);

	//////////////////////////////////////////////////////////////////////////
	T_RtlInitUnicodeString pRtlInitUnicodeString = nullptr;
	T_NtUnmapViewOfSection pNtUnmapViewOfSection = nullptr;
	T_NtOpenFile pNtOpenFile = nullptr;
	T_NtOpenDirectoryObject pNtOpenDirectoryObject = nullptr;
	T_NtOpenSection pNtOpenSection = nullptr;
	T_NtMapViewOfSection pNtMapViewOfSection = nullptr;
	T_RtlNtStatusToDosError pRtlNtStatusToDosError = nullptr;
	T_NtClose pNtClose = nullptr;
	T_NtCreateFile pNtCreateFile = nullptr;
	T_NtCreateSection pNtCreateSection = nullptr;
	T_NtQuerySection pNtQuerySection = nullptr;
	T_LdrLoadDll pLdrLoadDll = nullptr;
	T_RtlCompareUnicodeString pRtlCompareUnicodeString = nullptr;
	T_RtlPushFrame pRtlPushFrame = nullptr;
	T_RtlGetFrame pRtlGetFrame = nullptr;
	T_RtlPopFrame pRtlPopFrame = nullptr;
	T_NtQueryAttributesFile pNtQueryAttributesFile = nullptr;
	T_RtlDosPathNameToNtPathName_U pRtlDosPathNameToNtPathName_U = nullptr;
	//////////////////////////////////////////////////////////////////////////
	T_NtOpenFile OldNtOpenFile = nullptr;
	T_NtCreateSection OldNtCreateSection = nullptr;
	T_NtQuerySection OldNtQuerySection = nullptr;
	T_NtMapViewOfSection OldNtMapViewOfSection = nullptr;
	T_NtClose OldNtClose = nullptr;
	T_NtQueryAttributesFile OldNtQueryAttributesFile = nullptr;
	//////////////////////////////////////////////////////////////////////////
	struct LOAD_MEM_DLL_INFO : public ntdll::TEB_ACTIVE_FRAME
	{
		ULONG           xFlags;
		PVOID           MappedBase;
		PVOID           MemDllBase;
		SIZE_T          DllBufferSize;
		SIZE_T          ViewSize;
		ntdll::UNICODE_STRING  Lz32Path;

		union
		{
			HANDLE DllFileHandle;
			HANDLE SectionHandle;
		};

		ntdll::UNICODE_STRING  MemDllFullPath;
	};
#define _TAG4(s) ( \
	(((s) >> 24) & 0xFF)       | \
	(((s) >> 8 ) & 0xFF00)     | \
	(((s) << 24) & 0xFF000000) | \
	(((s) << 8 ) & 0x00FF0000) \
	)
#define TAG4(s) _TAG4((UINT32)(s))
	#define LOAD_MEM_DLL_INFO_MAGIC		TAG4('LDFM')

#if !defined(_M_IA64)
#define MEMORY_PAGE_SIZE (4 * 1024)
#else
#define MEMORY_PAGE_SIZE (8 * 1024)
#endif

	//////////////////////////////////////////////////////////////////////////
	NTSTATUS NTAPI OnNtOpenFile(
		OUT PHANDLE FileHandle,
		IN ACCESS_MASK DesiredAccess,
		IN ntdll::POBJECT_ATTRIBUTES ObjectAttributes,
		OUT ntdll::PIO_STATUS_BLOCK IoStatusBlock,
		IN ULONG ShareAccess,
		IN ULONG OpenOptions
	);
	NTSTATUS NTAPI OnNtCreateSection(
		OUT PHANDLE SectionHandle,
		IN ACCESS_MASK DesiredAccess,
		IN ntdll::POBJECT_ATTRIBUTES ObjectAttributes,  // Optional
		IN PLARGE_INTEGER MaximumSize,           // Optional
		IN ULONG SectionPageProtection,
		IN ULONG AllocationAttributes,
		IN HANDLE FileHandle                     // Optional
	);
	NTSTATUS NTAPI OnNtQuerySection(
		IN HANDLE SectionHandle,
		IN ntdll::SECTION_INFORMATION_CLASS SectionInformationClass,
		OUT PVOID SectionInformation,
		IN ULONG SectionInformationLength,
		OUT PULONG ResultLength OPTIONAL
	);

	NTSTATUS NTAPI OnNtMapViewOfSection(
		IN HANDLE  SectionHandle,
		IN HANDLE  ProcessHandle,
		IN OUT PVOID  *BaseAddress,
		IN ULONG  ZeroBits,
		IN ULONG  CommitSize,
		IN OUT PLARGE_INTEGER  SectionOffset,	// optional
		IN OUT PULONG  ViewSize,
		IN ntdll::SECTION_INHERIT  InheritDisposition,
		IN ULONG  AllocationType,
		IN ULONG  Protect
	);

	NTSTATUS NTAPI OnNtClose(
		IN HANDLE Handle
	);

	NTSTATUS NTAPI OnNtQueryAttributesFile(
		IN ntdll::POBJECT_ATTRIBUTES     ObjectAttributes,
		IN ntdll::PFILE_BASIC_INFORMATION FileInformation
	);
	//////////////////////////////////////////////////////////////////////////

	BOOL load_ldr_patch_apis()
	{
		auto hNtdll = GetModuleHandleW(L"ntdll.dll");

		if (!(pRtlInitUnicodeString = (T_RtlInitUnicodeString)GetProcAddress(hNtdll, "RtlInitUnicodeString"))) return FALSE;

		if (!(pNtUnmapViewOfSection = (T_NtUnmapViewOfSection)GetProcAddress(hNtdll, "NtUnmapViewOfSection")))	return FALSE;

		if (!(pNtOpenFile = (T_NtOpenFile)GetProcAddress(hNtdll, "NtOpenFile"))) return FALSE;

		if (!(pNtOpenDirectoryObject = (T_NtOpenDirectoryObject)GetProcAddress(hNtdll, "NtOpenDirectoryObject"))) return FALSE;

		if (!(pNtOpenSection = (T_NtOpenSection)GetProcAddress(hNtdll, "NtOpenSection"))) return FALSE;

		if (!(pNtMapViewOfSection = (T_NtMapViewOfSection)GetProcAddress(hNtdll, "NtMapViewOfSection"))) return FALSE;

		if (!(pRtlNtStatusToDosError = (T_RtlNtStatusToDosError)GetProcAddress(hNtdll, "RtlNtStatusToDosError"))) return FALSE;

		if (!(pNtClose = (T_NtClose)GetProcAddress(hNtdll, "NtClose"))) return FALSE;

		if (!(pNtCreateFile = (T_NtCreateFile)GetProcAddress(hNtdll, "NtCreateFile")))	return FALSE;

		if (!(pNtCreateSection = (T_NtCreateSection)GetProcAddress(hNtdll, "NtCreateSection"))) return FALSE;

		if (!(pNtQuerySection = (T_NtQuerySection)GetProcAddress(hNtdll, "NtQuerySection"))) return FALSE;

		if (!(pLdrLoadDll = (T_LdrLoadDll)GetProcAddress(hNtdll, "LdrLoadDll"))) return FALSE;

		if (!(pRtlCompareUnicodeString = (T_RtlCompareUnicodeString)GetProcAddress(hNtdll, "RtlCompareUnicodeString"))) return FALSE;

		if (!(pRtlPushFrame = (T_RtlPushFrame)GetProcAddress(hNtdll, "RtlPushFrame"))) return FALSE;

		if (!(pRtlGetFrame = (T_RtlGetFrame)GetProcAddress(hNtdll, "RtlGetFrame"))) return FALSE;

		if (!(pRtlPopFrame = (T_RtlPopFrame)GetProcAddress(hNtdll, "RtlPopFrame"))) return FALSE;

		if (!(pNtQueryAttributesFile = (T_NtQueryAttributesFile)GetProcAddress(hNtdll, "NtQueryAttributesFile"))) return FALSE;

		if (!(pRtlDosPathNameToNtPathName_U = (T_RtlDosPathNameToNtPathName_U)GetProcAddress(hNtdll, "RtlDosPathNameToNtPathName_U"))) return FALSE;

		return TRUE;
	}
	void install_ldr_patch()
	{
		
		{
			InterlockedIncrement(&_hookRef);
			if(b_hook)
				return;
		}
		ldr_lock();
		auto hNtdll = GetModuleHandleW(L"ntdll.dll");
		auto set_p = (pLdrSetDllManifestProber)GetProcAddress(hNtdll, "LdrSetDllManifestProber");
		/*if(set_p)
			set_p(nullptr, nullptr, nullptr);*/

		if (load_ldr_patch_apis())
		{
			InlineHook((void *)pNtOpenFile, (void *)OnNtOpenFile, (void **)&OldNtOpenFile);
			InlineHook((void *)pNtCreateSection, (void *)OnNtCreateSection, (void **)&OldNtCreateSection);
			InlineHook((void *)pNtQuerySection, (void *)OnNtQuerySection, (void **)&OldNtQuerySection);
			InlineHook((void *)pNtMapViewOfSection, (void *)OnNtMapViewOfSection, (void **)&OldNtMapViewOfSection);
			InlineHook((void *)pNtClose, (void *)OnNtClose, (void **)&OldNtClose);
			InlineHook((void *)pNtQueryAttributesFile, (void *)OnNtQueryAttributesFile, (void **)&OldNtQueryAttributesFile);
			b_hook = true;
		}
		else
		{
			DebugBreak();
		}
		ldr_unlock();
	}
	void uninstall_ldr_patch()
	{
		InterlockedDecrement(&_hookRef);
		if (_hookRef)
		{
			return;
		}
		ldr_lock();
		if (b_hook)
		{
			UnInlineHook((void *)pNtOpenFile, (void *)OldNtOpenFile);
			UnInlineHook((void *)pNtCreateSection, (void *)OldNtCreateSection);
			UnInlineHook((void *)pNtQuerySection, (void *)OldNtQuerySection);
			UnInlineHook((void *)pNtMapViewOfSection, (void *)OldNtMapViewOfSection);
			UnInlineHook((void *)pNtClose, (void *)OldNtClose);
			UnInlineHook((void *)pNtQueryAttributesFile, (void *)OldNtQueryAttributesFile);
		}
		b_hook = false;
		ldr_unlock();
	}
	//////////////////////////////////////////////////////////////////////////
	NTSTATUS
		LoadDllFromMemory(
			IN PVOID           DllBuffer,
			IN ULONG           DllBufferSize,
			IN ntdll::PUNICODE_STRING ModuleFileName,
			OUT PVOID*          ModuleHandle OPTIONAL,
			IN ULONG           Flags OPTIONAL
		)
	{
		NTSTATUS            Status = STATUS_UNSUCCESSFUL;
		PVOID               ModuleBase;
		LOAD_MEM_DLL_INFO   MemDllInfo;
		PIMAGE_DOS_HEADER   DosHeader;
		PIMAGE_NT_HEADERS   NtHeader;
		WCHAR               Lz32DosPath[MAX_PATH];
		WCHAR               SystemPath[MAX_PATH];
		ZeroMemory(&MemDllInfo, sizeof(MemDllInfo));
		MemDllInfo.Flags = LOAD_MEM_DLL_INFO_MAGIC;
		{
			pRtlPushFrame(&MemDllInfo);

			Status = STATUS_UNSUCCESSFUL;
			do
			{
				if (!pRtlDosPathNameToNtPathName_U(ModuleFileName->Buffer, &MemDllInfo.MemDllFullPath, NULL, NULL))
					break;

				/*Length = Nt_GetSystemDirectory(Lz32DosPath, countof(Lz32DosPath));
				*(PULONG64)(Lz32DosPath + Length)       = TAG4W('lz32');
				*(PULONG64)(Lz32DosPath + Length + 4)   = TAG4W('.dll');
				Lz32DosPath[Length + 8]                 = 0;*/
				GetSystemDirectoryW(SystemPath, MAX_PATH);
				wcsncpy_s(Lz32DosPath, SystemPath, MAX_PATH);
				wcsncat_s(Lz32DosPath, L"\\Lz32.dll", MAX_PATH);
				;//;//;//;//;//OutputDebugStringW(Lz32DosPath);
				if (!pRtlDosPathNameToNtPathName_U(Lz32DosPath, &MemDllInfo.Lz32Path, NULL, NULL))
					break;

				MemDllInfo.xFlags = Flags;
				MemDllInfo.MemDllBase = DllBuffer;
				MemDllInfo.DllBufferSize = DllBufferSize;
				DosHeader = (PIMAGE_DOS_HEADER)DllBuffer;
				NtHeader = (PIMAGE_NT_HEADERS)((ULONG_PTR)DllBuffer + DosHeader->e_lfanew);
				MemDllInfo.ViewSize = NtHeader->OptionalHeader.SizeOfImage;
				Status = pLdrLoadDll(NULL, 0, ModuleFileName, &ModuleBase);
				if (!NT_SUCCESS(Status))
				{
					break;
				}

				if (ModuleHandle != NULL)
					*ModuleHandle = (HANDLE)ModuleBase;
			} while (0);

			pRtlPopFrame(&MemDllInfo);
			return Status;
		}
		return STATUS_MEMORY_NOT_ALLOCATED;
	}

	BOOL LdrLoadMemDll(PVOID DllMem, SIZE_T dllSize, LPCWSTR szDllName, HMODULE *pModule)
	{
		{
			ntdll::UNICODE_STRING unDllName;
			ntdll::RtlInitUnicodeString(&unDllName, (PWSTR)szDllName);
			if (NT_SUCCESS(LoadDllFromMemory(DllMem, dllSize, &unDllName, (PVOID *)pModule, 0)))
			{
				return TRUE;
			}
		}
		return FALSE;
	}
	//////////////////////////////////////////////////////////////////////////
	ntdll::TEB_ACTIVE_FRAME *FindThreadFrameByContext(ULONG_PTR Context)
	{
		ntdll::TEB_ACTIVE_FRAME *Frame;

		Frame = pRtlGetFrame();
		while (Frame != NULL && (ULONG_PTR)Frame->Flags != Context)
			Frame = Frame->Previous;
		if (Frame)
		{
			OutputDebugString(_T("ok\r\n"));
		}
		return Frame;
	}

	LOAD_MEM_DLL_INFO* GetLoadMemDllInfo()
	{
		return (LOAD_MEM_DLL_INFO *)FindThreadFrameByContext(LOAD_MEM_DLL_INFO_MAGIC);
	}
	//////////////////////////////////////////////////////////////////////////
	NTSTATUS NTAPI OnNtOpenFile(
		OUT PHANDLE FileHandle,
		IN ACCESS_MASK DesiredAccess,
		IN ntdll::POBJECT_ATTRIBUTES ObjectAttributes,
		OUT ntdll::PIO_STATUS_BLOCK IoStatusBlock,
		IN ULONG ShareAccess,
		IN ULONG OpenOptions
	)
	{
		NTSTATUS            Status;
		LOAD_MEM_DLL_INFO  *MemDllInfo;

		MemDllInfo = GetLoadMemDllInfo();
		if (MemDllInfo == NULL ||
			pRtlCompareUnicodeString(ObjectAttributes->ObjectName, &MemDllInfo->MemDllFullPath, TRUE))
		{
			return OldNtOpenFile(FileHandle, DesiredAccess, ObjectAttributes, IoStatusBlock, ShareAccess, OpenOptions);
		}

		ObjectAttributes->ObjectName = &MemDllInfo->Lz32Path;
		Status = OldNtOpenFile(FileHandle, DesiredAccess, ObjectAttributes, IoStatusBlock, ShareAccess, OpenOptions);
		if (NT_SUCCESS(Status))
		{
			MemDllInfo->DllFileHandle = *FileHandle;
		}
		return Status;
	}
	NTSTATUS NTAPI OnNtCreateSection(
		OUT PHANDLE SectionHandle,
		IN ACCESS_MASK DesiredAccess,
		IN ntdll::POBJECT_ATTRIBUTES ObjectAttributes,  // Optional
		IN PLARGE_INTEGER MaximumSize,           // Optional
		IN ULONG SectionPageProtection,
		IN ULONG AllocationAttributes,
		IN HANDLE FileHandle                     // Optional
	)
	{
		BOOL                IsDllHandle;
		NTSTATUS            Status;
		LARGE_INTEGER       SectionSize;
		LOAD_MEM_DLL_INFO  *MemDllInfo;

		IsDllHandle = FALSE;
		MemDllInfo = NULL;

		if (FileHandle != NULL)
		{
			MemDllInfo = GetLoadMemDllInfo();
			if (MemDllInfo != NULL && MemDllInfo->DllFileHandle == FileHandle)
			{
				//            if (MaximumSize == NULL)
				MaximumSize = &SectionSize;

				MaximumSize->QuadPart = MemDllInfo->ViewSize;
				DesiredAccess = SECTION_MAP_READ | SECTION_MAP_WRITE | SECTION_MAP_EXECUTE;
				SectionPageProtection = PAGE_EXECUTE_READWRITE;
				AllocationAttributes = SEC_COMMIT;
				FileHandle = NULL;
				IsDllHandle = TRUE;
			}
		}

		Status = OldNtCreateSection(
			SectionHandle,
			DesiredAccess,
			ObjectAttributes,
			MaximumSize,
			SectionPageProtection,
			AllocationAttributes,
			FileHandle
		);

		if (!NT_SUCCESS(Status) || !IsDllHandle)
		{
			return Status;
		}

		MemDllInfo->SectionHandle = *SectionHandle;

		return Status;
	}

	NTSTATUS NTAPI OnNtQuerySection(
		IN HANDLE SectionHandle,
		IN ntdll::SECTION_INFORMATION_CLASS SectionInformationClass,
		OUT PVOID SectionInformation,
		IN ULONG SectionInformationLength,
		OUT PULONG ResultLength OPTIONAL
	)
	{
		PIMAGE_DOS_HEADER           DosHeader;
		PIMAGE_NT_HEADERS           NtHeaders;
		PIMAGE_OPTIONAL_HEADER      OptionalHeader;
		LOAD_MEM_DLL_INFO          *MemDllInfo;
		ntdll::SECTION_IMAGE_INFORMATION  *ImageInfo;
		ntdll::SECTION_BASIC_INFORMATION  *BasicInfo;

		MemDllInfo = GetLoadMemDllInfo();
		if (MemDllInfo == NULL || SectionHandle == NULL || MemDllInfo->SectionHandle != SectionHandle)
			goto DEFAULT_PROC;

		DosHeader = (PIMAGE_DOS_HEADER)MemDllInfo->MemDllBase;
		NtHeaders = (PIMAGE_NT_HEADERS)((ULONG_PTR)DosHeader + DosHeader->e_lfanew);
		OptionalHeader = &NtHeaders->OptionalHeader;

		switch (SectionInformationClass)
		{
		case ntdll::SectionBasicInformation:
			BasicInfo = (ntdll::SECTION_BASIC_INFORMATION *)SectionInformation;
			BasicInfo->BaseAddress = MemDllInfo->MappedBase;
			BasicInfo->AllocationAttributes = 0;
			BasicInfo->MaximumSize.QuadPart = MemDllInfo->ViewSize;
			break;

		case ntdll::SectionImageInformation:
			if (ResultLength != NULL)
				*ResultLength = sizeof(*ImageInfo);

			if (SectionInformationLength < sizeof(*ImageInfo))
				return STATUS_BUFFER_TOO_SMALL;

			if (SectionInformation == NULL)
				break;

			ImageInfo = (ntdll::SECTION_IMAGE_INFORMATION *)SectionInformation;
			ImageInfo->TransferAddress = (PVOID)((ULONG_PTR)DosHeader + OptionalHeader->AddressOfEntryPoint);
			ImageInfo->ZeroBits = 0;
			ImageInfo->MaximumStackSize = OptionalHeader->SizeOfStackReserve;
			ImageInfo->CommittedStackSize = OptionalHeader->SizeOfStackCommit;
			ImageInfo->SubSystemType = OptionalHeader->Subsystem;
			ImageInfo->SubSystemMinorVersion = OptionalHeader->MinorSubsystemVersion;
			ImageInfo->SubSystemMajorVersion = OptionalHeader->MajorSubsystemVersion;
			ImageInfo->OperatingSystemVersion = 0;
			ImageInfo->ImageCharacteristics = NtHeaders->FileHeader.Characteristics;
			ImageInfo->DllCharacteristics = OptionalHeader->DllCharacteristics;
			ImageInfo->Machine = NtHeaders->FileHeader.Machine;
			ImageInfo->ImageContainsCode = 0; // OptionalHeader->SizeOfCode;
			ImageInfo->LoaderFlags = OptionalHeader->LoaderFlags;
			ImageInfo->ImageFileSize = MemDllInfo->DllBufferSize;
			ImageInfo->CheckSum = OptionalHeader->CheckSum;
			break;

		case ntdll::SectionRelocationInformation:
			if (SectionInformation != NULL)
				*(PULONG_PTR)SectionInformation = (ULONG_PTR)MemDllInfo->MappedBase - (ULONG_PTR)OptionalHeader->ImageBase;

			if (ResultLength != NULL)
				*ResultLength = sizeof(ULONG_PTR);

			break;

		default:
			goto DEFAULT_PROC;
		}

		return STATUS_SUCCESS;

	DEFAULT_PROC:
		return OldNtQuerySection(SectionHandle, SectionInformationClass, SectionInformation, SectionInformationLength, (PSIZE_T)ResultLength);
	}
	NTSTATUS NTAPI OnNtMapViewOfSection(
		IN HANDLE  SectionHandle,
		IN HANDLE  ProcessHandle,
		IN OUT PVOID  *BaseAddress,
		IN ULONG  ZeroBits,
		IN ULONG  CommitSize,
		IN OUT PLARGE_INTEGER  SectionOffset,	// optional
		IN OUT PULONG  ViewSize,
		IN ntdll::SECTION_INHERIT  InheritDisposition,
		IN ULONG  AllocationType,
		IN ULONG  Protect
	)
	{
		NTSTATUS                    Status;
		LOAD_MEM_DLL_INFO          *MemDllInfo;
		PIMAGE_DOS_HEADER           DosHeader;
		PIMAGE_NT_HEADERS           NtHeader;
		PIMAGE_SECTION_HEADER       SectionHeader;
		PBYTE                       DllBase, ModuleBase;
		ULONG						NewViewSize;
		if (SectionHandle == NULL)
			goto CALL_OLD_PROC;

		MemDllInfo = GetLoadMemDllInfo();
		if (MemDllInfo == NULL)
			goto CALL_OLD_PROC;

		if (SectionHandle != MemDllInfo->SectionHandle)
			goto CALL_OLD_PROC;

		if (SectionOffset != NULL)
			SectionOffset->QuadPart = 0;
		if (ViewSize == NULL)
		{
			ViewSize = &NewViewSize;
		}
		//if(ViewSize)
		*ViewSize = MemDllInfo->ViewSize;

		Status = OldNtMapViewOfSection(
			SectionHandle,
			ProcessHandle,
			BaseAddress,
			0,
			0,
			NULL,
			(PSIZE_T)ViewSize,
			ntdll::ViewShare,
			0,
			PAGE_EXECUTE_READWRITE
		);
		if (!NT_SUCCESS(Status))
			return Status;

		MemDllInfo->MappedBase = *BaseAddress;

		ModuleBase = (PBYTE)*BaseAddress;
		DllBase = (PBYTE)MemDllInfo->MemDllBase;

		{
			DosHeader = (PIMAGE_DOS_HEADER)DllBase;
			NtHeader = (PIMAGE_NT_HEADERS)((ULONG_PTR)DllBase + DosHeader->e_lfanew);
			SectionHeader = (PIMAGE_SECTION_HEADER)((ULONG_PTR)&NtHeader->OptionalHeader + NtHeader->FileHeader.SizeOfOptionalHeader);
			for (ULONG NumberOfSections = NtHeader->FileHeader.NumberOfSections; NumberOfSections; ++SectionHeader, --NumberOfSections)
			{
				CopyMemory(
					ModuleBase + SectionHeader->VirtualAddress,
					DllBase + SectionHeader->PointerToRawData,
					SectionHeader->SizeOfRawData
				);
			}

			//CopyMemory(ModuleBase, DllBase, (ULONG_PTR)SectionHeader - (ULONG_PTR)DllBase);
			CopyMemory(ModuleBase, DllBase, MEMORY_PAGE_SIZE);
		}

		Status = (ULONG_PTR)ModuleBase != NtHeader->OptionalHeader.ImageBase ? STATUS_IMAGE_NOT_AT_BASE : STATUS_SUCCESS;
		return Status;

	CALL_OLD_PROC:
		return OldNtMapViewOfSection(SectionHandle, ProcessHandle, BaseAddress, ZeroBits, CommitSize, SectionOffset, (PSIZE_T)ViewSize, InheritDisposition, AllocationType, Protect);
	}
	NTSTATUS NTAPI OnNtClose(
		IN HANDLE Handle
	)
	{
		LOAD_MEM_DLL_INFO *MemDllInfo;

		MemDllInfo = GetLoadMemDllInfo();
		if (MemDllInfo != NULL && Handle != NULL)
		{
			if (MemDllInfo->DllFileHandle == Handle)
				MemDllInfo->DllFileHandle = NULL;
			else if (MemDllInfo->SectionHandle == Handle)
				MemDllInfo->SectionHandle = NULL;
		}
		return OldNtClose(Handle);
	}

	NTSTATUS NTAPI OnNtQueryAttributesFile(
		IN ntdll::POBJECT_ATTRIBUTES     ObjectAttributes,
		IN ntdll::PFILE_BASIC_INFORMATION FileInformation
	)
	{
		LOAD_MEM_DLL_INFO  *MemDllInfo;

		MemDllInfo = GetLoadMemDllInfo();

		if (MemDllInfo == NULL || pRtlCompareUnicodeString(ObjectAttributes->ObjectName, &MemDllInfo->MemDllFullPath, TRUE))
		{
			goto PassThr;
		}

		return STATUS_SUCCESS;
	PassThr:
		return OldNtQueryAttributesFile(ObjectAttributes, FileInformation);
	}
	//////////////////////////////////////////////////////////////////////////
}