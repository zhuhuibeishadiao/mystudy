#pragma once
#include "stdafx.h"
namespace usr::ldr
{
	class ldr_module
	{
	public:
		ldr_module(DWORD ProcessId)
		{
			local_image_base = nullptr;
			process_ = usr::util::Process(ProcessId);
		};
		ldr_module() {
			local_image_base = nullptr;
		};
		~ldr_module() {
			if (local_image_base)
			{
				VirtualFree(local_image_base,0,MEM_RELEASE);
			}
		};
		void no_release()
		{
			local_image_base = nullptr;
		}
		PVOID load_module(PVOID _image, std::size_t _size=0)
		{
			if (!LoadImage(_image, local_image_base))
			{
				return nullptr;
			}
			auto nt_header = ntdll::RtlImageNtHeader(local_image_base);
			auto image_base = nt_header->OptionalHeader.ImageBase;
			if (process_.get() != GetCurrentProcess())
			{
				//远程
				auto mem = usr::util::Memory(process_);
				remote_image_base = mem.alloc_mem(_image_size);
				LdrInitGSCookie(local_image_base);
				ProcessRelocs(local_image_base, DWORD_PTR(remote_image_base) - image_base);
				ProcessImport(local_image_base);	
				mem.write(remote_image_base, local_image_base, _image_size);
				return remote_image_base;
			}
			else
			{
				ProcessRelocs(local_image_base, DWORD_PTR(local_image_base) - image_base);
				ProcessImport(local_image_base);
				LdrInitGSCookie(local_image_base);
				return local_image_base;
			}
		}
		PVOID get_oep() {
			if (!local_image_base)
			{
				return nullptr;
			}
			auto nt_header = ntdll::RtlImageNtHeader(local_image_base);
			auto ep_rva = nt_header->OptionalHeader.AddressOfEntryPoint;
			if (process_.get() != GetCurrentProcess())
			{
				return PVOID((DWORD_PTR)remote_image_base + ep_rva);
			}
			else
			{
				return PVOID((DWORD_PTR)local_image_base + ep_rva);
			}
		}
		PVOID load_module(std::vector<BYTE> image) {
			if (!LoadImage(image.data(), local_image_base))
			{
				return nullptr;
			}
			auto nt_header = ntdll::RtlImageNtHeader(local_image_base);
			auto image_base = nt_header->OptionalHeader.ImageBase;
			if (process_.get() != GetCurrentProcess())
			{
				//远程
				auto mem = usr::util::Memory(process_);
				remote_image_base = mem.alloc_mem(_image_size);
				LdrInitGSCookie(local_image_base);
				ProcessRelocs(local_image_base, DWORD_PTR(remote_image_base) - image_base);
				ProcessImport(local_image_base);
				
				mem.write(remote_image_base, local_image_base, _image_size);
				return remote_image_base;
			}
			else
			{
				LdrInitGSCookie(local_image_base);
				ProcessRelocs(local_image_base, DWORD_PTR(local_image_base) - image_base);
				ProcessImport(local_image_base);
				return local_image_base;
			}
		};
	private:
		usr::util::Process process_;
		PVOID local_image_base;
		PVOID remote_image_base;
		DWORD _image_size;
	private:
		BOOL ProcessRelocs(PVOID pvImageBase, DWORD_PTR dwDelta)
		{
			DWORD dwRelocsSize;
			PIMAGE_BASE_RELOCATION pReloc;
			if (dwDelta)
			{
				pReloc = (PIMAGE_BASE_RELOCATION)ntdll::RtlImageDirectoryEntryToData(pvImageBase, TRUE, IMAGE_DIRECTORY_ENTRY_BASERELOC, &dwRelocsSize);
				if (pReloc && dwRelocsSize)
				{
					PIMAGE_BASE_RELOCATION pEndReloc = (PIMAGE_BASE_RELOCATION)(pReloc + dwRelocsSize);

					while (pReloc->SizeOfBlock && pReloc < pEndReloc)
					{
						pReloc = ntdll::LdrProcessRelocationBlock(MAKE_PTR(pvImageBase, pReloc->VirtualAddress, ULONG_PTR), (pReloc->SizeOfBlock - sizeof(*pReloc)) / sizeof(USHORT), (PUSHORT)(pReloc + 1), dwDelta);
						if (!pReloc) return FALSE;
					}
				}
			}

			return TRUE;
		}
		BOOLEAN ProcessImport(PVOID pvImageBase)
		{
			DWORD dwImportSize;
			PIMAGE_IMPORT_DESCRIPTOR pImport;

			pImport = (PIMAGE_IMPORT_DESCRIPTOR)ntdll::RtlImageDirectoryEntryToData(pvImageBase, TRUE, IMAGE_DIRECTORY_ENTRY_IMPORT, &dwImportSize);
			if (pImport && dwImportSize)
			{
				for (; pImport->Name; pImport++)
				{
					PCHAR szDllName = RVA_TO_VA(pvImageBase, pImport->Name);
					HMODULE hDll = LoadLibraryA(szDllName);
					if (!hDll) return FALSE;

					PDWORD_PTR thunkRef, funcRef;

					if (pImport->OriginalFirstThunk)
					{
						thunkRef = MAKE_PTR(pvImageBase, pImport->OriginalFirstThunk, PDWORD_PTR);
						funcRef = MAKE_PTR(pvImageBase, pImport->FirstThunk, PDWORD_PTR);
					}
					else
					{
						thunkRef = MAKE_PTR(pvImageBase, pImport->FirstThunk, PDWORD_PTR);
						funcRef = MAKE_PTR(pvImageBase, pImport->FirstThunk, PDWORD_PTR);
					}

					for (; *thunkRef; thunkRef++, funcRef++)
					{
						PVOID pvProcAddress;

						if (IMAGE_SNAP_BY_ORDINAL(*thunkRef))
						{
							pvProcAddress = GetProcAddress(hDll, (PCHAR)IMAGE_ORDINAL(*thunkRef));
						}
						else
						{
							pvProcAddress = GetProcAddress(hDll, (PCHAR)&((PIMAGE_IMPORT_BY_NAME)RVA_TO_VA(pvImageBase, *thunkRef))->Name);
						}
						if (!pvProcAddress) return FALSE;

						*(PVOID*)funcRef = pvProcAddress;
					}
				}
			}

			return TRUE;
		}
		BOOL LoadImage(PVOID _data, PVOID &pvImageBase) {
			auto b_load = FALSE;
			auto dos_header = reinterpret_cast<PIMAGE_DOS_HEADER>(_data);
			do
			{
				if (dos_header->e_magic != IMAGE_DOS_SIGNATURE)
				{
					std::cout << "failed dos sig" << std::endl;
					break;
				}
				auto nt_header = reinterpret_cast<PIMAGE_NT_HEADERS32>(reinterpret_cast<char *>(dos_header) + dos_header->e_lfanew);
				if (nt_header->Signature != IMAGE_NT_SIGNATURE)
				{
					std::cout << "failed nt sig " << nt_header->Signature << std::endl;
					break;
				}
				if (nt_header->OptionalHeader.SectionAlignment & 1)
				{
					std::cout << "alignment size is 1" << std::endl;
					break;
				}

				SYSTEM_INFO sysInfo;
				GetNativeSystemInfo(&sysInfo);
				auto image_size = ALIGN_UP(nt_header->OptionalHeader.SizeOfImage, sysInfo.dwPageSize);
				image_size = ALIGN_UP(image_size, nt_header->OptionalHeader.SectionAlignment);
				auto section_header = IMAGE_FIRST_SECTION(nt_header);
				for (int i = 0; i < nt_header->FileHeader.NumberOfSections; ++i)
				{
					//得到该节的大小
					auto nCodeSize = section_header[i].Misc.VirtualSize;
					auto nLoadSize = section_header[i].SizeOfRawData;
					auto nMaxSize = (nLoadSize > nCodeSize) ? (nLoadSize) : (nCodeSize);
					auto nSectionSize = ALIGN_UP(section_header[i].VirtualAddress + nMaxSize, sysInfo.dwPageSize);

					if (image_size < nSectionSize)
					{
						image_size = nSectionSize;  //Use the Max;
					}
				}

				pvImageBase = VirtualAlloc(nullptr, image_size, MEM_COMMIT|MEM_RESERVE, PAGE_EXECUTE_READWRITE);
				auto buff_byte = reinterpret_cast<BYTE*>(_data);
				{
					int  nHeaderSize = nt_header->OptionalHeader.SizeOfHeaders;
					int  nSectionSize = nt_header->FileHeader.NumberOfSections * sizeof(IMAGE_SECTION_HEADER);
					int  nMoveSize = nHeaderSize + nSectionSize;
					RtlCopyMemory(pvImageBase, buff_byte, nMoveSize);
				}
				{
					for (auto i = 0; i < nt_header->FileHeader.NumberOfSections; ++i)
					{
						if (section_header[i].VirtualAddress == 0 || section_header[i].SizeOfRawData == 0)
						{
							continue;
						}
						// 定位该节在内存中的位置
						void *pSectionAddress = (void *)((PBYTE)pvImageBase + section_header[i].VirtualAddress);
						// 复制段数据到虚拟内存
						RtlCopyMemory(pSectionAddress, &(buff_byte[section_header[i].PointerToRawData]), section_header[i].SizeOfRawData);
					}
				}
				_image_size = image_size;
				b_load = TRUE;
			} while (0);
			return b_load;
		}
		void LdrInitGSCookie(PVOID ImageBase)
		{
			ULONG size_ = 0;
			auto nt_header = ntdll::RtlImageNtHeader(ImageBase);
			auto image_base = nt_header->OptionalHeader.ImageBase;
			auto _is64 = [nt_header]() {
				if (nt_header->FileHeader.Machine==LIEF::PE::IMAGE_FILE_MACHINE_AMD64)
				{
					return true;
				}
				return false;
			};
			auto section = ntdll::RtlImageDirectoryEntryToData(ImageBase, TRUE, IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG, &size_);
			auto sec64 = reinterpret_cast<PIMAGE_LOAD_CONFIG_DIRECTORY64>(section);
			auto sec32 = reinterpret_cast<PIMAGE_LOAD_CONFIG_DIRECTORY32>(section);
			FILETIME systime = { 0 };
			LARGE_INTEGER PerformanceCount = { { 0 } };
			uintptr_t cookie = 0;

			GetSystemTimeAsFileTime(&systime);
			QueryPerformanceCounter(&PerformanceCount);

			cookie = GetCurrentProcessId() ^ GetCurrentThreadId() ^ reinterpret_cast<uintptr_t>(&cookie);

#ifdef _WIN64
			cookie ^= *reinterpret_cast<uint64_t*>(&systime);
			cookie ^= (PerformanceCount.QuadPart << 32) ^ PerformanceCount.QuadPart;
			cookie &= 0xFFFFFFFFFFFF;

			if (cookie == 0x2B992DDFA232)
				cookie++;
#else

			cookie ^= systime.dwHighDateTime ^ systime.dwLowDateTime;
			cookie ^= PerformanceCount.LowPart;
			cookie ^= PerformanceCount.HighPart;

			if (cookie == 0xBB40E64E)
				cookie++;
			else if (!(cookie & 0xFFFF0000))
				cookie |= (cookie | 0x4711) << 16;
#endif
			if (!_is64())
			{
				//sec->SecurityCookie
				if (sec32 && sec32->SecurityCookie)
				{
					auto rva = sec32->SecurityCookie - (DWORD)ImageBase;
					auto write_address = sec32->SecurityCookie;
					*(uintptr_t *)(write_address) = cookie;
				}
			}
			else
			{
				if (sec64 && sec64->SecurityCookie)
				{
					auto rva = sec64->SecurityCookie - (DWORD_PTR)ImageBase;
					auto write_address = sec64->SecurityCookie;
					*(uintptr_t *)(write_address) = cookie;
				}
			}
		}
	};
}