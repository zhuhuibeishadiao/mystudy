#include "stdafx.h"
namespace ddk::util
{
	PVOID load_dll(std::wstring filename)
	{
		HANDLE hSection, hFile;
		UNICODE_STRING dllName;
		PVOID BaseAddress = NULL;
		SIZE_T size = 0;
		NTSTATUS stat;
		OBJECT_ATTRIBUTES oa = { sizeof(oa), 0, &dllName, OBJ_CASE_INSENSITIVE };
		IO_STATUS_BLOCK iosb;
		auto full_dll_path = filename.c_str();
		LOG_DEBUG("DBG: ABout to load %ws at IRQL %d\n", full_dll_path,
			KeGetCurrentIrql());
		RtlInitUnicodeString(&dllName, full_dll_path);


		//_asm int 3;
		stat = ZwOpenFile(&hFile, FILE_EXECUTE | SYNCHRONIZE, &oa, &iosb,
			FILE_SHARE_READ, FILE_SYNCHRONOUS_IO_NONALERT);

		if (!NT_SUCCESS(stat)) {
			LOG_DEBUG("WRN: Can't open %ws: %x\n", full_dll_path, stat);
			return 0;
		}

		oa.ObjectName = 0;

		stat = ZwCreateSection(&hSection, SECTION_ALL_ACCESS, &oa, 0, PAGE_EXECUTE,
			SEC_IMAGE, hFile);

		if (!NT_SUCCESS(stat)) {
			LOG_DEBUG("WRN: Can't create section %ws: %x\n", full_dll_path, stat);
			return 0;
		}

		stat = ZwMapViewOfSection(hSection, NtCurrentProcess(), &BaseAddress, 0,
			1000, 0, &size, (SECTION_INHERIT)1, MEM_TOP_DOWN, PAGE_READWRITE);

		if (!NT_SUCCESS(stat)) {
			LOG_DEBUG("WRN: Can't map section %ws: %x\n", full_dll_path, stat);
			return 0;
		}

		ZwClose(hSection);
		ZwClose(hFile);

		LOG_DEBUG("DBG: Successfully loaded %ws\n", full_dll_path);
		return BaseAddress;
	}
	VOID free_dll(HANDLE hMod)
	{
		ZwUnmapViewOfSection(NtCurrentProcess(), hMod);
	}
	PVOID get_sysinfo(SYSTEM_INFORMATION_CLASS InfoClass)
	{
		NTSTATUS ns;
		ULONG RetSize, Size = 0x1100;
		PVOID Info;

		while (1)
		{
			if ((Info = malloc(Size)) == NULL)
			{
				return NULL;
			}

			RetSize = 0;
			ns = ZwQuerySystemInformation(InfoClass, Info, Size, &RetSize);
			if (ns == STATUS_INFO_LENGTH_MISMATCH)
			{
				free(Info);
				Info = NULL;

				if (RetSize > 0)
				{
					Size = RetSize + 0x1000;
				}
				else
					break;
			}
			else
				break;
		}

		if (!NT_SUCCESS(ns))
		{
			if (Info)
				free(Info);

			return NULL;
		}
		return Info;
	}

	bool get_sys_module_list(std::vector<AUX_MODULE_EXTENDED_INFO> &syslist)
	{
		ULONG modulesSize = 0;
		AUX_MODULE_EXTENDED_INFO*  modules;
		ULONG  numberOfModules;
		auto status = AuxKlibInitialize();
		LOG_DEBUG("AuxKlibInitialize return %x\r\n",status);
		if (NT_SUCCESS(status))
		{
			status = AuxKlibQueryModuleInformation(&modulesSize, sizeof(AUX_MODULE_EXTENDED_INFO), NULL);
			LOG_DEBUG("AuxKlibQueryModuleInformation return %x\r\n",status);
			if (NT_SUCCESS(status))
			{
				LOG_DEBUG("modulesSize %d\r\n",modulesSize);
				if (modulesSize > 0)
				{
					numberOfModules = modulesSize / sizeof(AUX_MODULE_EXTENDED_INFO);
					modules = (AUX_MODULE_EXTENDED_INFO*)malloc(modulesSize);
					auto mem_exit = std::experimental::make_scope_exit([&]() {if (modules)free(modules); });
					if (modules != NULL)
					{
						status = AuxKlibQueryModuleInformation(&modulesSize, sizeof(AUX_MODULE_EXTENDED_INFO), modules);
						LOG_DEBUG("AuxKlibQueryModuleInformation return %x\r\n",status);

						if (NT_SUCCESS(status))
						{
							for (ULONG i = 0; i < numberOfModules; i++)
							{
								syslist.push_back(modules[i]);
							}
							return true;
						}
					}
				}
			}
		}
		return false;
	}
	PVOID get_module_base(std::string modulename, PULONG_PTR pImageSize)
	{
		std::vector<AUX_MODULE_EXTENDED_INFO> syslist;
		UNICODE_STRING usCommonHalName, usCommonNtName;
		RtlInitUnicodeString(&usCommonHalName, L"hal.dll");
		RtlInitUnicodeString(&usCommonNtName, L"ntoskrnl.exe");
		if (get_sys_module_list(syslist))
		{
#define HAL_NAMES_NUM 6
			wchar_t *wcHalNames[] =
			{
				L"hal.dll",      // Non-ACPI PIC HAL 
				L"halacpi.dll",  // ACPI PIC HAL
				L"halapic.dll",  // Non-ACPI APIC UP HAL
				L"halmps.dll",   // Non-ACPI APIC MP HAL
				L"halaacpi.dll", // ACPI APIC UP HAL
				L"halmacpi.dll"  // ACPI APIC MP HAL
			};

#define NT_NAMES_NUM 4
			wchar_t *wcNtNames[] =
			{
				L"ntoskrnl.exe", // UP
				L"ntkrnlpa.exe", // UP PAE
				L"ntkrnlmp.exe", // MP
				L"ntkrpamp.exe"  // MP PAE
			};

			ANSI_STRING asModuleName;
			UNICODE_STRING usModuleName;
			NTSTATUS ns;
			RtlInitAnsiString(&asModuleName, modulename.c_str());
			ns = RtlAnsiStringToUnicodeString(&usModuleName, &asModuleName, TRUE);
			if (NT_SUCCESS(ns))
			{
				auto exit_us_name = std::experimental::make_scope_exit([&]() {RtlFreeUnicodeString(&usModuleName); });

				for (auto n = size_t(0); n < syslist.size(); n++)
				{
					ANSI_STRING asEnumModuleName;
					UNICODE_STRING usEnumModuleName;
					RtlInitAnsiString(
						&asEnumModuleName,
						(char *)syslist[n].FullPathName + syslist[n].FileNameOffset
					);

					ns = RtlAnsiStringToUnicodeString(&usEnumModuleName, &asEnumModuleName, TRUE);
					if (NT_SUCCESS(ns))
					{
						auto exit_us_name2 = std::experimental::make_scope_exit([&]() {RtlFreeUnicodeString(&usEnumModuleName); });

						if (RtlEqualUnicodeString(&usModuleName, &usCommonHalName, TRUE))
						{
							int i_m = 0;
							for (i_m = 0; i_m < HAL_NAMES_NUM; i_m++)
							{
								UNICODE_STRING usHalName;
								RtlInitUnicodeString(&usHalName, wcHalNames[i_m]);
								if (RtlEqualUnicodeString(&usEnumModuleName, &usHalName, TRUE))
								{
									if (pImageSize)
									{
										*pImageSize = (ULONG_PTR)syslist[n].ImageSize;
									}
									return reinterpret_cast<PVOID>(syslist[n].BasicInfo.ImageBase);
								}
							}
						}
						else if (RtlEqualUnicodeString(&usModuleName, &usCommonNtName, TRUE))
						{
							int i_m = 0;
							for (i_m = 0; i_m < NT_NAMES_NUM; i_m++)
							{
								UNICODE_STRING usNtName;
								RtlInitUnicodeString(&usNtName, wcNtNames[i_m]);
								if (RtlEqualUnicodeString(&usEnumModuleName, &usNtName, TRUE))
								{
									if (pImageSize)
									{
										*pImageSize = (ULONG_PTR)syslist[n].ImageSize;
									}
									return reinterpret_cast<PVOID>(syslist[n].BasicInfo.ImageBase);
								}
							}
						}
						else if (RtlEqualUnicodeString(&usModuleName, &usEnumModuleName, TRUE))
						{
							if (pImageSize)
							{
								*pImageSize = (ULONG_PTR)syslist[n].ImageSize;
							}
							return reinterpret_cast<PVOID>(syslist[n].BasicInfo.ImageBase);
						}

					}
				}
			}
		}
		return nullptr;
	}
	//////////////////////////////////////////////////////////////////////////
	bool getPdbInfo(PVOID ImageBase, std::string &pdbFileName, std::string &symSignature)
	{
		ULONG nsize = 0;
		auto pDebug = RtlImageDirectoryEntryToData(ImageBase, TRUE, IMAGE_DIRECTORY_ENTRY_DEBUG, &nsize);
		if (pDebug)
		{
			auto image_base = reinterpret_cast<char *>(ImageBase);
			auto dbg_header = reinterpret_cast<PIMAGE_DEBUG_DIRECTORY>(pDebug);
			for (unsigned int i = 0; i < nsize / sizeof(IMAGE_DEBUG_DIRECTORY); i++)
			{
				if (dbg_header[i].Type == IMAGE_DEBUG_TYPE_CODEVIEW)
				{
					auto cvinfo = reinterpret_cast<CV_HEADER *>(image_base + dbg_header[i].AddressOfRawData);
					if (cvinfo->CvSignature == CV_SIGNATURE_NB10)
					{
						auto p_cv_info = reinterpret_cast<CV_INFO_PDB20 *>(image_base + dbg_header[i].AddressOfRawData);
						LOG_DEBUG("pdbfilename = %s\r\n", reinterpret_cast<char *>(p_cv_info->PdbFileName));
						continue;
					}
					if (cvinfo->CvSignature == CV_SIGNATURE_RSDS)
					{
						CHAR szSymSignature[65] = { 0 };
						auto pCvData = reinterpret_cast<CV_INFO_PDB70 *>(image_base + dbg_header[i].AddressOfRawData);
						RtlStringCchPrintfA(szSymSignature, 64,
							"%08X%04X%04X%02hX%02hX%02hX%02hX%02hX%02hX%02hX%02hX%d",
							pCvData->Signature.Data1, pCvData->Signature.Data2,
							pCvData->Signature.Data3, pCvData->Signature.Data4[0],
							pCvData->Signature.Data4[1], pCvData->Signature.Data4[2],
							pCvData->Signature.Data4[3], pCvData->Signature.Data4[4],
							pCvData->Signature.Data4[5], pCvData->Signature.Data4[6],
							pCvData->Signature.Data4[7], pCvData->Age);
						//std::cout << "pdb filename = " << reinterpret_cast<char *>(pCvData->PdbFileName) << std::endl;
						//std::cout << "pdb sig = " << szSymSignature << std::endl;
						LOG_DEBUG("filename=%s pdb sig=%s\r\n", reinterpret_cast<char *>(pCvData->PdbFileName), szSymSignature);
						auto pdb_name = reinterpret_cast<char *>(pCvData->PdbFileName);
						pdbFileName = std::string(pdb_name);
						symSignature = szSymSignature;
						return true;
					}
				}
				if (dbg_header[i].Type == IMAGE_DEBUG_TYPE_MISC)
				{
					auto dbg_misc = reinterpret_cast<IMAGE_DEBUG_MISC *>(image_base + dbg_header[i].AddressOfRawData);
					//std::cout << "pdb filename = " << reinterpret_cast<char*>(dbg_misc->Data) << std::endl;
					LOG_DEBUG("pdb filename = %s\r\n", reinterpret_cast<char*>(dbg_misc->Data));
					continue;
				}
			}
		}
		return false;
	}
	//////////////////////////////////////////////////////////////////////////
	ULONG_PTR get_proc_address(PVOID Image, std::string functionname)
	{
		auto RVATOVA = [](auto _base_, auto _offset_){
			return ((PUCHAR)(_base_)+(ULONG)(_offset_));
		};
		__try
		{
			PIMAGE_EXPORT_DIRECTORY pExport = NULL;

			PIMAGE_NT_HEADERS32 pHeaders32 = (PIMAGE_NT_HEADERS32)
				((PUCHAR)Image + ((PIMAGE_DOS_HEADER)Image)->e_lfanew);

			if (pHeaders32->FileHeader.Machine == IMAGE_FILE_MACHINE_I386)
			{
				// 32-bit image
				if (pHeaders32->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress)
				{
					pExport = (PIMAGE_EXPORT_DIRECTORY)RVATOVA(
						Image,
						pHeaders32->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress
					);
				}
			}
			else if (pHeaders32->FileHeader.Machine == IMAGE_FILE_MACHINE_AMD64)
			{
				// 64-bit image
				PIMAGE_NT_HEADERS64 pHeaders64 = (PIMAGE_NT_HEADERS64)
					((PUCHAR)Image + ((PIMAGE_DOS_HEADER)Image)->e_lfanew);

				if (pHeaders64->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress)
				{
					pExport = (PIMAGE_EXPORT_DIRECTORY)RVATOVA(
						Image,
						pHeaders64->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress
					);
				}
			}

			if (pExport)
			{
				PULONG AddressOfFunctions = (PULONG)RVATOVA(Image, pExport->AddressOfFunctions);
				PSHORT AddrOfOrdinals = (PSHORT)RVATOVA(Image, pExport->AddressOfNameOrdinals);
				PULONG AddressOfNames = (PULONG)RVATOVA(Image, pExport->AddressOfNames);
				ULONG i = 0;
				for (i = 0; i < pExport->NumberOfFunctions; i++)
				{
					if (!strcmp((char *)RVATOVA(Image, AddressOfNames[i]), functionname.c_str()))
					{
						return AddressOfFunctions[AddrOfOrdinals[i]];
					}
				}
			}
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{

		}
		return 0;
	}
	//////////////////////////////////////////////////////////////////////////
	NTSTATUS LoadFileToMem(wchar_t *strFileName, PVOID *lpVirtualAddress)
	{
		NTSTATUS				Status;
		HANDLE					hFile;
		LARGE_INTEGER			FileOffset;
		UNICODE_STRING			usFileName;
		OBJECT_ATTRIBUTES		ObjAttr;
		IO_STATUS_BLOCK			IoStatusBlock;

		IMAGE_DOS_HEADER		ImageDosHeader;
		IMAGE_NT_HEADERS		ImageNtHeader;
		IMAGE_SECTION_HEADER	*pImageSectionHeader;

		ULONG					uIndex;
		PVOID					lpVirtualPointer;
		ULONG					SecVirtualAddress, SizeOfSection;
		ULONG					PointerToRawData;

		if (!MmIsAddressValid(strFileName))
		{
			return STATUS_UNSUCCESSFUL;
		}

		RtlInitUnicodeString(&usFileName, strFileName);

		InitializeObjectAttributes(\
			&ObjAttr, \
			&usFileName, \
			OBJ_CASE_INSENSITIVE, \
			NULL, \
			NULL);

		Status = ZwCreateFile(\
			&hFile, \
			FILE_ALL_ACCESS, \
			&ObjAttr, \
			&IoStatusBlock, \
			NULL, \
			FILE_ATTRIBUTE_NORMAL, \
			FILE_SHARE_READ, \
			FILE_OPEN, \
			FILE_NON_DIRECTORY_FILE, \
			NULL, \
			0);
		if (!NT_SUCCESS(Status))
		{
			LOG_DEBUG("ZwCreateFile failed:%X\r\n", Status);
			return Status;
		}

		FileOffset.QuadPart = 0;
		Status = ZwReadFile(\
			hFile, \
			NULL, \
			NULL, \
			NULL, \
			&IoStatusBlock, \
			&ImageDosHeader, \
			sizeof(IMAGE_DOS_HEADER), \
			&FileOffset, \
			NULL);
		if (!NT_SUCCESS(Status))
		{
			LOG_DEBUG("read iamge_dos_header failed:%X\r\n", Status);
			ZwClose(hFile);
			return Status;
		}

		FileOffset.QuadPart = ImageDosHeader.e_lfanew;
		Status = ZwReadFile(\
			hFile, \
			NULL, \
			NULL, \
			NULL, \
			&IoStatusBlock, \
			&ImageNtHeader, \
			sizeof(IMAGE_NT_HEADERS), \
			&FileOffset, \
			NULL);
		if (!NT_SUCCESS(Status))
		{
			LOG_DEBUG("read IMAGE_NT_HEADERS failed:%X\r\n", Status);
			ZwClose(hFile);
			return Status;
		}

		pImageSectionHeader = (IMAGE_SECTION_HEADER *)ExAllocatePool(\
			NonPagedPool, \
			sizeof(IMAGE_SECTION_HEADER)*ImageNtHeader.FileHeader.NumberOfSections);
		if (pImageSectionHeader == 0)
		{
			LOG_DEBUG("pImageSectionHeader is null.\r\n");
			ZwClose(hFile);
			return STATUS_UNSUCCESSFUL;
		}

		FileOffset.QuadPart += sizeof(IMAGE_NT_HEADERS);
		Status = ZwReadFile(\
			hFile, \
			NULL, \
			NULL, \
			NULL, \
			&IoStatusBlock, \
			pImageSectionHeader, \
			sizeof(IMAGE_SECTION_HEADER)*ImageNtHeader.FileHeader.NumberOfSections, \
			&FileOffset, \
			NULL);
		if (!NT_SUCCESS(Status))
		{
			LOG_DEBUG("read IMAGE_SECTION_HEADER failed:%X\r\n", Status);
			ExFreePool(pImageSectionHeader);
			ZwClose(hFile);
			return Status;
		}

		lpVirtualPointer = ExAllocatePool(NonPagedPool, ImageNtHeader.OptionalHeader.SizeOfImage);
		if (lpVirtualPointer == 0)
		{
			LOG_DEBUG("lpVirtualPointer is null\r\n");
			ExFreePool(pImageSectionHeader);
			ZwClose(hFile);
			return STATUS_UNSUCCESSFUL;
		}

		memset(lpVirtualPointer, 0, ImageNtHeader.OptionalHeader.SizeOfImage);
		RtlCopyMemory(\
			lpVirtualPointer, \
			&ImageDosHeader, \
			sizeof(IMAGE_DOS_HEADER));
		RtlCopyMemory(\
			(PVOID)((ULONG_PTR)lpVirtualPointer + ImageDosHeader.e_lfanew), \
			&ImageNtHeader, \
			sizeof(IMAGE_NT_HEADERS));
		RtlCopyMemory(\
			(PVOID)((ULONG_PTR)lpVirtualPointer + ImageDosHeader.e_lfanew + sizeof(IMAGE_NT_HEADERS)), \
			pImageSectionHeader,
			sizeof(IMAGE_SECTION_HEADER)*ImageNtHeader.FileHeader.NumberOfSections);

		for (uIndex = 0; uIndex < ImageNtHeader.FileHeader.NumberOfSections; uIndex++)
		{
			SecVirtualAddress = pImageSectionHeader[uIndex].VirtualAddress;
			SizeOfSection = max(pImageSectionHeader[uIndex].SizeOfRawData, \
				pImageSectionHeader[uIndex].Misc.VirtualSize);

			PointerToRawData = pImageSectionHeader[uIndex].PointerToRawData;

			FileOffset.QuadPart = PointerToRawData;
			Status = ZwReadFile(\
				hFile, \
				NULL, \
				NULL, \
				NULL, \
				&IoStatusBlock, \
				(PVOID)((ULONG_PTR)lpVirtualPointer + SecVirtualAddress), \
				SizeOfSection, \
				&FileOffset, \
				NULL);
			if (!NT_SUCCESS(Status))
			{
				LOG_DEBUG("read failed is pImageSectionHeader[%d]\r\n", uIndex);
				ExFreePool(pImageSectionHeader);
				ExFreePool(lpVirtualPointer);
				ZwClose(hFile);
				return Status;
			}
		}

		LOG_DEBUG("ok!\r\n");

		ExFreePool(pImageSectionHeader);
		*lpVirtualAddress = lpVirtualPointer;
		ZwClose(hFile);
		return STATUS_SUCCESS;
	}
	NTSTATUS GetSystemImageInfo(char* __in pszImageName, RTL_PROCESS_MODULE_INFORMATION* __out lpsystemModule)
	{
		NTSTATUS		st;
		ULONG			count;
		ULONG_PTR		sizeOfBuf;
		char			aszImageName[256];
		PRTL_PROCESS_MODULES	lpsystemModuleInfo;

		if (MmIsAddressValid(pszImageName) == FALSE ||
			MmIsAddressValid(lpsystemModule) == FALSE)
		{
			return STATUS_UNSUCCESSFUL;
		}

		sizeOfBuf = 0;
		memset(aszImageName, 0, 256);
		auto pFin = std::experimental::make_unique_resource(
			ddk::util::get_sysinfo(SystemModuleInformation), &free);

		lpsystemModuleInfo = reinterpret_cast<PRTL_PROCESS_MODULES>(pFin.get());
		if (!lpsystemModuleInfo)
		{
			return STATUS_UNSUCCESSFUL;
		}
		st = STATUS_UNSUCCESSFUL;
		for (count = 0; count < lpsystemModuleInfo->NumberOfModules; count++)
		{
			strcpy(aszImageName, (CCHAR*)lpsystemModuleInfo->Modules[count].FullPathName);
			_strlwr(aszImageName);
			_strlwr(pszImageName);

			if (strstr(aszImageName, pszImageName) != 0)
			{
				__try {
					*lpsystemModule = lpsystemModuleInfo->Modules[count];
				}
				__except (EXCEPTION_EXECUTE_HANDLER) {
					break;
				}

				st = STATUS_SUCCESS;
				break;
			}
		}
		return st;
	}
	//////////////////////////////////////////////////////////////////////////
	ULONG GetPreviousModeOffset()
	{
		auto PrevModeOffset = 0UL;
		auto fnExGetPreviousMode = get_proc_address("ExGetPreviousMode");
		if (fnExGetPreviousMode)
		{
			LOG_DEBUG("ExGetPreviousMode %p\r\n", fnExGetPreviousMode);
			UCHAR PreviousModePattern[] = "\x00\x00\xC3";
			auto pFound = ddk::mem_util::MmMemMem((const void*)fnExGetPreviousMode, 32, PreviousModePattern, sizeof(PreviousModePattern) - 1);
			if (pFound)
				PrevModeOffset = *(DWORD *)((PUCHAR)pFound - 2);
		}
		return PrevModeOffset;
	}
	PVOID get_proc_address(std::string functionname)
	{
		ANSI_STRING asName;
		UNICODE_STRING usName;
		NTSTATUS ns;
		RtlInitAnsiString(&asName, functionname.c_str());
		ns = RtlAnsiStringToUnicodeString(&usName, &asName, TRUE);
		if (NT_SUCCESS(ns))
		{
			LOG_DEBUG("%wZ\r\n", &usName);
			auto exit_us_name = std::experimental::make_scope_exit([&] {RtlFreeUnicodeString(&usName); });
			auto pfn = MmGetSystemRoutineAddress(&usName);
			if (pfn)
			{
				return pfn;
			}
		}
		return nullptr;
	}
	//////////////////////////////////////////////////////////////////////////
	PKSERVICE_TABLE_DESCRIPTOR get_ssdt()
	{
		static PKSERVICE_TABLE_DESCRIPTOR Ssdt = nullptr;
		if (Ssdt)
		{
			return Ssdt;
		}
		PVOID ptrKiSystemCall64 = NULL;
		ptrKiSystemCall64 = (PVOID)__readmsr(0xC0000082);
		UCHAR PTRN_WALL_Ke[] = { 0x42, 0x3b, 0x44, 0x17, 0x10, 0x0f, 0x83 };
		LONG OFFS_WNO8_Ke = -0x19;
		LONG OFFS_WIN8_Ke = -0x16;
		LONG OFFS_WIN10_Ke = -0x27;
		auto ns = ddk::mem_util::MmGenericPointerSearch((PUCHAR *)&Ssdt,
			((PUCHAR)ptrKiSystemCall64) - (1 * PAGE_SIZE),
			((PUCHAR)ptrKiSystemCall64) + (1 * PAGE_SIZE),
			PTRN_WALL_Ke,
			sizeof(PTRN_WALL_Ke),
			(OsIndex < OsIndex_8) ? OFFS_WNO8_Ke : (OsIndex < OsIndex_10_1607) ? OFFS_WIN8_Ke : OFFS_WIN10_Ke);

		if (NT_SUCCESS(ns))
		{
			LOG_DEBUG("SSDT %p\r\n", Ssdt);
			return Ssdt;
		}
		return nullptr;
	}
	PVOID get_ssdt_function_address(DWORD index)
	{
		auto SystemTable = get_ssdt();
		if (index == DWORD(-1))
		{
			return nullptr;
		}
		auto OldFunction = (ULONG_PTR)SystemTable->OffsetToService;
		if (OsIndex<OsIndex_VISTA)
		{
#ifdef _M_IX86
#define EX_FAST_REF_MASK	0x07
#else
#define EX_FAST_REF_MASK	0x0f
#endif
			OldFunction += SystemTable->OffsetToService[index] & ~EX_FAST_REF_MASK;
			//	NewOffset = ((LONG)(Function - (ULONG_PTR)KeServiceDescriptorTable->OffsetToService)) | EX_FAST_REF_MASK + KeServiceDescriptorTable->OffsetToService[ssdtNumber] & EX_FAST_REF_MASK;
		}
		else
		{
			OldFunction += SystemTable->OffsetToService[index] >> 4;
			//NewOffset = (((LONG)(Function - (ULONG_PTR)KeServiceDescriptorTable->OffsetToService)) << 4) + KeServiceDescriptorTable->OffsetToService[ssdtNumber] & 0x0F;
		}
		return reinterpret_cast<PVOID>(OldFunction);
	}

	PFILE_OBJECT get_file_object_by_fullimagename(PUNICODE_STRING usFileName)
	{
		//通过打开FullPath
		PFILE_OBJECT file_object = nullptr;
		OBJECT_ATTRIBUTES oa = { 0 };
		InitializeObjectAttributes(&oa, usFileName,
			OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, NULL, NULL);

		IO_STATUS_BLOCK iosb = { 0 };
		HANDLE  FileHandle = NULL;
		auto ns = ZwOpenFile(&FileHandle,
			GENERIC_READ | SYNCHRONIZE,
			&oa, &iosb,
			FILE_SHARE_READ,
			FILE_SYNCHRONOUS_IO_NONALERT | FILE_NON_DIRECTORY_FILE);
		if (NT_SUCCESS(ns)) 
		{
			ns = ObReferenceObjectByHandle(FileHandle,
				0,
				*IoFileObjectType,
				KernelMode,
				(PVOID *)&file_object,
				NULL);
			ZwClose(FileHandle);
			FileHandle = NULL;
		}
		else {

			// xp下， 如果路径是\WINDOWS\SYSTEM32\kernel32.dll这样的格式
			// 就只能通过这种方式才解决了

			__try {

				file_object = CONTAINING_RECORD(usFileName, FILE_OBJECT, FileName);

				if (file_object->Type != 5) {
					//FILE_OBJECT的这个值是固定的
					file_object = nullptr;
				}

			}
			__except (EXCEPTION_EXECUTE_HANDLER) {

				file_object = NULL;
			}
			if (file_object)
			{
				ObReferenceObject(file_object);
			}
		}
		return file_object;
	}

	//////////////////////////////////////////////////////////////////////////
	bool get_file_object_full_path(PFILE_OBJECT fileobject, PUNICODE_STRING usFullPath)
	{
		//XP以后的系统上IoQueryXXX获得才是真的
		//if (KeGetCurrentIrql()<=PASSIVE_LEVEL)
		//{
		//  //IoQueryFileDosDeviceName蓝屏太严重并不随时的使用
		//	POBJECT_NAME_INFORMATION object_name_info = nullptr;
		//	auto ns = SAFE_NATIVE_CALL(IoQueryFileDosDeviceName,fileobject, &object_name_info);
		//	if (NT_SUCCESS(ns)
		//		&& object_name_info)
		//	{
		//		RtlCopyUnicodeString(usFullPath, &object_name_info->Name);
		//		ExFreePool(object_name_info);
		//		return true;
		//	}
		//}
		//vista之前的系统只有26个盘下面代码暴力获取可行
		ULONG ReturnLength = 0;
		bool relate_name = false;
		auto relate_file_object = fileobject->RelatedFileObject;
		auto IsValidUnicodeString = [](PUNICODE_STRING pstr)->decltype(relate_name) {
			bool bRc = false;
			ULONG   ulIndex = 0;
			__try
			{
				if (!MmIsAddressValid(pstr))
					return false;

				if ((NULL == pstr->Buffer) || (0 == pstr->Length))
					return false;

				for (ulIndex = 0; ulIndex < pstr->Length; ulIndex++)
				{
					if (!MmIsAddressValid((UCHAR *)pstr->Buffer + ulIndex))
						return false;
				}

				bRc = true;
			}
			__except (EXCEPTION_EXECUTE_HANDLER)
			{
				bRc = false;
			}
			return bRc;
		};

		auto is_file_object_name = IsValidUnicodeString(&fileobject->FileName);
		auto is_relate_object_name = relate_file_object ? IsValidUnicodeString(&relate_file_object->FileName) : false;

		if (is_relate_object_name && is_file_object_name)
		{
			if (fileobject->FileName.Buffer[0] != L'\\')
				relate_name = true;
		}
		if (is_file_object_name)
		{
			NTSTATUS status;
			DEVICE_OBJECT* VolumeDeviceObject = NULL;
			if (fileobject->Vpb != NULL &&
				fileobject->Vpb->RealDevice != NULL) {
				VolumeDeviceObject = fileobject->Vpb->RealDevice;
			}
			else {
				VolumeDeviceObject = fileobject->DeviceObject;
			}
			status = ObQueryNameString(VolumeDeviceObject, NULL, 0, &ReturnLength);
			if (ReturnLength == 0) {
				return false;
			}
			POBJECT_NAME_INFORMATION NameInfo =
				(POBJECT_NAME_INFORMATION)malloc(ReturnLength);
			if (!NameInfo)
			{
				return false;
			}
			auto free_nameinfo = std::experimental::make_scope_exit([&]() {free(NameInfo); });
			status = ObQueryNameString(VolumeDeviceObject,
				(POBJECT_NAME_INFORMATION)NameInfo,
				ReturnLength,
				&ReturnLength);
			if (NT_SUCCESS(status))
			{
				//\Device\HarddiskVolume2\Windows\System32\notepad.exe
				UNICODE_STRING* usDriverName = &NameInfo->Name;
				UNICODE_STRING usSymbolName = { 0 };
				WCHAR SymbolBuffer[16] = { L"\\??\\X:" };
				RtlInitUnicodeString(&usSymbolName, SymbolBuffer);
				for (WCHAR c = L'A'; c < (L'Z' + 1); ++c)
				{
					usSymbolName.Buffer[wcslen(L"\\??\\")] = c;
					OBJECT_ATTRIBUTES oa;
					InitializeObjectAttributes(
						&oa,
						&usSymbolName,
						OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE,
						NULL, NULL);

					HANDLE hSymbol;
					status = ZwOpenSymbolicLinkObject(
						&hSymbol,
						GENERIC_READ,
						&oa);

					if (!NT_SUCCESS(status)) {
						continue;
					}

					WCHAR TargetBuffer[64] = { 0 };
					UNICODE_STRING usTarget = { 0 };
					RtlInitEmptyUnicodeString(&usTarget, TargetBuffer, sizeof(TargetBuffer));

					ReturnLength = 0;
					status = ZwQuerySymbolicLinkObject(hSymbol, &usTarget, &ReturnLength);

					ZwClose(hSymbol);
					hSymbol = NULL;

					if (NT_SUCCESS(status)) {

						if (0 == RtlCompareUnicodeString(usDriverName, &usTarget, FALSE)) {

							RtlCopyUnicodeString(usFullPath, &usSymbolName);
							if (relate_name)
							{
								if (relate_file_object->FileName.Buffer[0] != L'\\'
									&& (L'\0' != relate_file_object->FileName.Buffer[0]))
								{
									RtlAppendUnicodeToString(usFullPath, L"\\");
								}
								RtlAppendUnicodeStringToString(usFullPath, &relate_file_object->FileName);
							}
							if (fileobject->FileName.Buffer[0] != L'\\'
								&&fileobject->FileName.Buffer[0] != L'\0')
							{
								RtlAppendUnicodeToString(usFullPath, L"\\");
							}
							RtlAppendUnicodeStringToString(usFullPath, &fileobject->FileName);
							return true;
						}
					}
				}
			}
		}
		return false;
	}
	bool get_file_full_path(PUNICODE_STRING inPath, PUNICODE_STRING outPath)
	{
		OBJECT_ATTRIBUTES oa = { 0 };
		InitializeObjectAttributes(&oa, inPath,
			OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, NULL, NULL);

		IO_STATUS_BLOCK iosb = { 0 };
		HANDLE  FileHandle = NULL;
		auto ns = ZwOpenFile(&FileHandle,
			GENERIC_READ | SYNCHRONIZE,
			&oa, &iosb,
			FILE_SHARE_READ,
			FILE_SYNCHRONOUS_IO_NONALERT);
		if (NT_SUCCESS(ns)) {
			PFILE_OBJECT file_object = nullptr;
			ns = ObReferenceObjectByHandle(FileHandle,
				0,
				*IoFileObjectType,
				KernelMode,
				(PVOID *)&file_object,
				NULL);
			ZwClose(FileHandle);
			FileHandle = NULL;
			if (NT_SUCCESS(ns))
			{
				auto ret = get_file_object_full_path(file_object, outPath);
				ObDereferenceObject(file_object);
				return ret;
			}
		}
		return false;
	}
	//////////////////////////////////////////////////////////////////////////
	BOOLEAN get_reg_fullname(
		PUNICODE_STRING pRegistryPath,
		PUNICODE_STRING pPartialRegistryPath,
		PVOID pRegistryObject)
	{
		BOOLEAN foundCompleteName = FALSE;
		BOOLEAN partial = FALSE;
		if ((!MmIsAddressValid(pRegistryObject)) ||
			(pRegistryObject == NULL))
		{
			return FALSE;
		}
		if (pPartialRegistryPath != NULL)
		{
			if ((((pPartialRegistryPath->Buffer[0] == '\\')
				|| (pPartialRegistryPath->Buffer[0] == '%'))
				|| ((pPartialRegistryPath->Buffer[0] == 'T')
					&& (pPartialRegistryPath->Buffer[1] == 'R')
					&& (pPartialRegistryPath->Buffer[2] == 'Y')
					&& (pPartialRegistryPath->Buffer[3] == '\\'))))
			{
				RtlUnicodeStringCopy(pRegistryPath, pPartialRegistryPath);
				partial = TRUE;
				foundCompleteName = TRUE;
			}
		}
		if (!foundCompleteName)
		{
			ULONG returnedLength = 0;
			auto status = ObQueryNameString(pRegistryObject, nullptr, 0, &returnedLength);
			if (status == STATUS_INFO_LENGTH_MISMATCH)
			{
				auto pObjectName = reinterpret_cast<PUNICODE_STRING>(malloc(returnedLength));
				status = ObQueryNameString(pRegistryObject, (POBJECT_NAME_INFORMATION)pObjectName, returnedLength, &returnedLength);
				if (NT_SUCCESS(status))
				{
					RtlUnicodeStringCopy(pRegistryPath, pObjectName);
					foundCompleteName = TRUE;
				}
				free(pObjectName);
			}
		}
		return foundCompleteName;
	}
};

namespace ddk::util
{
	bool get_driver_object(std::wstring drvname, PDRIVER_OBJECT &drv_object)
	{
		UNICODE_STRING nsDrvName;
		RtlInitUnicodeString(&nsDrvName, drvname.c_str());
		auto ns = ObReferenceObjectByName(&nsDrvName,
			OBJ_CASE_INSENSITIVE,
			nullptr,
			0,
			*IoDriverObjectType,
			KernelMode,
			nullptr,
			reinterpret_cast<PVOID*>(&drv_object));
		if (NT_SUCCESS(ns))
		{
			return true;
		}
		return false;
	}
}

namespace ddk::util
{
	bool nt_mkdir(std::wstring _dirname)
	{
		UNICODE_STRING usDirName = {};
		OBJECT_ATTRIBUTES oa = {};
		HANDLE _dir1 = nullptr;
		IO_STATUS_BLOCK ios = {};
		RtlInitUnicodeString(&usDirName, _dirname.c_str());

		InitializeObjectAttributes(&oa, &usDirName, 
			OBJ_CASE_INSENSITIVE|OBJ_KERNEL_HANDLE, NULL,
			NULL);

		auto ns = ZwCreateFile(&_dir1, FILE_WRITE_ATTRIBUTES, &oa, &ios, NULL,
			FILE_ATTRIBUTE_NORMAL,
			FILE_SHARE_READ | FILE_SHARE_WRITE, FILE_OPEN_IF,
			FILE_DIRECTORY_FILE,
			NULL, 0);

		if (!NT_SUCCESS(ns))
		{
			return false;
		}
		ZwClose(_dir1);
		return true;
	}
	//////////////////////////////////////////////////////////////////////////
	ULONG KeQueryActiveProcessorCountCompatible()
	{
#if (NTDDI_VERSION >= NTDDI_VISTA)
		return KeQueryActiveProcessorCountEx(ALL_PROCESSOR_GROUPS);
#else
		ULONG numberOfProcessors = 0;
		auto affinity = KeQueryActiveProcessors();
		for (; affinity; affinity >>= 1)
		{
			if (affinity & 1)
			{
				numberOfProcessors++;
			}
		}
		return numberOfProcessors;
#endif
	}
	NTSTATUS ForEachProcessors(
		__in _EachCallBackRoutine CallbackRoutine,
		__in_opt void* Context)
	{
		const auto number_of_processors = KeQueryActiveProcessorCountEx(ALL_PROCESSOR_GROUPS);
		for (auto processor_index = 0ul; 
			processor_index < number_of_processors;
			processor_index++) {
			PROCESSOR_NUMBER processor_number = {};
			auto status =
				KeGetProcessorNumberFromIndex(processor_index, &processor_number);
			if (!NT_SUCCESS(status)) {
				return status;
			}
			GROUP_AFFINITY affinity = {};
			affinity.Group = processor_number.Group;
			affinity.Mask = 1ull << processor_number.Number;
			GROUP_AFFINITY previous_affinity = {};
			KeSetSystemGroupAffinityThread(&affinity, &previous_affinity);
			status = CallbackRoutine(Context);
			KeRevertToUserGroupAffinityThread(&previous_affinity);
			if (!NT_SUCCESS(status)) {
				return status;
			}
		}
		return STATUS_SUCCESS;
	}

	//////////////////////////////////////////////////////////////////////////
	ULONG get_syscall_number(std::string functionname)
	{
		auto NtdllBase = ddk::util::load_dll(std::wstring(L"\\SystemRoot\\System32\\ntdll.dll"));
		auto exit_1 = std::experimental::make_scope_exit([&]() {
			if (NtdllBase)
				ddk::util::free_dll(NtdllBase); });
		if (!NtdllBase)
		{
			return ULONG(-1);
		}
		auto rva = get_proc_address(NtdllBase, functionname);
		if (rva)
		{
				PUCHAR Func = (PUCHAR)NtdllBase + rva;
#ifdef _X86_
				// check for mov eax,imm32
				if (*Func == 0xB8)
				{
					// return imm32 argument (syscall numbr)
					return *(PULONG)((PUCHAR)Func + 1);
				}
#elif _AMD64_
				// check for mov eax,imm32
				if (*(Func + 3) == 0xB8)
				{
					// return imm32 argument (syscall numbr)
					return *(PULONG)(Func + 4);
				}
#endif
		}
		return ULONG(-1);
	}

	//////////////////////////////////////////////////////////////////////////
	SIZE_T AlignSize(SIZE_T nSize, UINT nAlign)
	{
		if (nAlign == 0)
		{
			return nSize;
		}
		return ((nSize + nAlign - 1) / nAlign * nAlign);
	}
	//////////////////////////////////////////////////////////////////////////
	PVOID get_ntos_imagebase()
	{
		PVOID imageBase = nullptr;
		auto p_function = ddk::util::get_proc_address("PsGetCurrentProcessId");
		RtlPcToFileHeader(p_function, &imageBase);
		return imageBase;
	}

};