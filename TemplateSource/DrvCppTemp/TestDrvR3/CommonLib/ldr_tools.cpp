#include "stdafx.h"

namespace usr::ldr::tools
{
	void free_dll(HANDLE hMod)
	{
		ntdll::ZwUnmapViewOfSection(GetCurrentProcess(), hMod);
	}
	PVOID load_dll(std::wstring filename)
	{
		using namespace ntdll;
		HANDLE hSection, hFile;
		UNICODE_STRING dllName;
		PVOID BaseAddress = NULL;
		SIZE_T size = 0;
		OBJECT_ATTRIBUTES oa = { sizeof(oa), 0, &dllName, OBJ_CASE_INSENSITIVE };
		IO_STATUS_BLOCK iosb;
		auto full_dll_path = filename.c_str();

		RtlInitUnicodeString(&dllName, (PWSTR)full_dll_path);

		auto stat = ZwOpenFile(&hFile, FILE_EXECUTE | SYNCHRONIZE, &oa, &iosb,
			FILE_SHARE_READ, FILE_SYNCHRONOUS_IO_NONALERT);

		if (stat<0) {
			DbgPrint("WRN: Can't open %ws: %x\n", full_dll_path, stat);
			return 0;
		}

		oa.ObjectName = 0;

		stat = ZwCreateSection(&hSection, SECTION_ALL_ACCESS, &oa, 0, PAGE_EXECUTE,
			SEC_IMAGE, hFile);

		if (stat<0) {
			DbgPrint("WRN: Can't create section %ws: %x\n", full_dll_path, stat);
			return 0;
		}

		stat = ZwMapViewOfSection(hSection, GetCurrentProcess(), &BaseAddress, 0,
			1000, 0, &size, SECTION_INHERIT::ViewShare, MEM_TOP_DOWN, PAGE_READWRITE);

		if (stat<0) {
			DbgPrint("WRN: Can't map section %ws: %x\n", full_dll_path, stat);
			return 0;
		}

		ZwClose(hSection);
		ZwClose(hFile);

		DbgPrint("DBG: Successfully loaded %ws\n", full_dll_path);
		return BaseAddress;
	};

	PVOID get_module_handle_process(DWORD_PTR ProcessId, 
		LPCTSTR lpszModuleName,
		BOOL bGet32Module)
	{

		auto snapshot = std::experimental::make_unique_resource(
			CreateToolhelp32Snapshot(bGet32Module ? TH32CS_SNAPMODULE32
				| TH32CS_SNAPMODULE : TH32CS_SNAPMODULE, DWORD(ProcessId)),
			&CloseHandle);
		auto hSnap = snapshot.get();
		if (!hSnap || hSnap == INVALID_HANDLE_VALUE)
		{
			ntdll::DbgPrint("SnapFailed\r\n");
			return nullptr;
		}
		MODULEENTRY32W me32 = {};
		me32.dwSize = sizeof(MODULEENTRY32W);
		if (!Module32First(hSnap, &me32))
		{
			ntdll::DbgPrint("Can not GetXX\r\n");
			return nullptr;
		}

		do
		{
			if (_tcsicmp(me32.szModule, lpszModuleName) == 0)
			{
				if (bGet32Module)
				{
					if (_tcsstr(_tcslwr(me32.szExePath), L"\\syswow64\\"))
					{
						return PVOID(me32.modBaseAddr);
					}
				}
				else
					return PVOID(me32.modBaseAddr);
			}
			RtlZeroMemory(&me32, sizeof(me32));
			me32.dwSize = sizeof(MODULEENTRY32W);
		} while (Module32Next(hSnap, &me32));

		return nullptr;
	}

	ULONG_PTR get_proc_address(PVOID Image, LPCSTR functionname)
	{
#define RVATOVA(_base_, _offset_) ((PUCHAR)(_base_) + (ULONG)(_offset_))
		__try
		{
			PIMAGE_EXPORT_DIRECTORY pExport = NULL;

			PIMAGE_NT_HEADERS32 pHeaders32 = (PIMAGE_NT_HEADERS32)
				((PUCHAR)Image + ((PIMAGE_DOS_HEADER)Image)->e_lfanew);

			if (pHeaders32->FileHeader.Machine == LIEF::PE::IMAGE_FILE_MACHINE_I386)
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
			else if (pHeaders32->FileHeader.Machine == LIEF::PE::IMAGE_FILE_MACHINE_AMD64)
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
					if (!strcmp((char *)RVATOVA(Image, AddressOfNames[i]), functionname))
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
	PVOID get_pro_address_process(
		DWORD_PTR ProcessId,
		LPCTSTR lpszModuleName,
		LPCSTR lpszApi,
		BOOL bGet32)
	{

		std::wstring dll_name = std::wstring(L"\\SystemRoot\\System32\\");
		if (bGet32)
		{
			dll_name = std::wstring(L"\\SystemRoot\\SysWow64\\");
		}
		dll_name += std::wstring(lpszModuleName);
		auto pBase = get_module_handle_process(ProcessId, lpszModuleName, bGet32);
		if (!pBase)
		{
			return nullptr;
		}
		auto dll = std::experimental::make_unique_resource(
			load_dll(dll_name), &free_dll
		);
		auto pImageBase = dll.get();
		if (!pImageBase)
		{
			return nullptr;
		}
		auto ulRVA = get_proc_address(pImageBase, lpszApi);
		if (!ulRVA)
		{
			return nullptr;
		}
		auto pRet = PVOID((PUCHAR)pBase + ulRVA);
		return pRet;
	};
	PVOID get_pro_address_process(DWORD_PTR ProcessId,
		LPCTSTR lpszModuleName,
		LPCSTR lpszApi)
	{
		auto proc = usr::util::Process(ProcessId);
		auto cur = usr::util::Process();
		if (cur.is_process64())
		{
			if (!proc.is_process64())
			{
				return get_pro_address_process(ProcessId, lpszModuleName, lpszApi, TRUE);
			}
		}
		return get_pro_address_process(ProcessId, lpszModuleName, lpszApi, FALSE);
	}

	PVOID get_section_by_name(PVOID ImageBase, std::string sec_name, SIZE_T &_size)
	{
		auto nt_header = ntdll::RtlImageNtHeader(ImageBase);
		if (nt_header)
		{
			auto section_header = IMAGE_FIRST_SECTION(nt_header);
			if (section_header)
			{
				for (auto i=0;i<nt_header->FileHeader.NumberOfSections;i++)
				{
					auto section_hdr = &section_header[i];
					if (_stricmp((PCHAR)section_hdr->Name, sec_name.c_str()) == 0)
					{
						_size = section_hdr->Misc.VirtualSize;
						return PVOID((PBYTE)ImageBase + section_hdr->VirtualAddress);
					}
				}
			}
		}
		return nullptr;
	}
	PVOID get_peb_ldr_by_hmodule(PVOID hmodule)
	{
		ntdll::PROCESS_BASIC_INFORMATION stInfo = { 0 };
		DWORD dwRetnLen = 0;
		DWORD dw = ntdll::NtQueryInformationProcess(GetCurrentProcess(), ntdll::ProcessBasicInformation, &stInfo, sizeof(stInfo), &dwRetnLen);
		ntdll::PPEB pPeb = (ntdll::PPEB)stInfo.PebBaseAddress;
		PLIST_ENTRY ListHead, Current;
		ntdll::LDR_DATA_TABLE_ENTRY *pstEntry = NULL;

		ListHead = &(((ntdll::PPEB)stInfo.PebBaseAddress)->Ldr->InMemoryOrderModuleList);
		Current = ListHead->Flink;
		while (Current != ListHead)
		{
			pstEntry = CONTAINING_RECORD(Current, ntdll::LDR_DATA_TABLE_ENTRY, InMemoryOrderLinks);
			if (pstEntry->DllBase==hmodule)
			{
				return (PVOID)pstEntry;
			}
			Current = pstEntry->InMemoryOrderLinks.Flink;
		}
		return nullptr;
	}
}