#pragma once
#include <windows.h>
#include <string>
#include "../CommonCPP/Common.h"

namespace frog::util {
	class FileUtils
	{
		//public with constructor and deconstructor
	public:
		FileUtils() {}
		FileUtils(const WCHAR *path)
		{
			CopyMemory(m_path, path, min((wcslen(path) + 1) * 2, MAX_PATH * 2));
			DoMap();
		}
		~FileUtils() {
			DoUnMap();
		}

		//public with common tool
	public:
		//映射path路径的文件到内存
		PVOID mapFile(const WCHAR *path)
		{
			DoUnMap();
			CopyMemory(m_path, path, min((wcslen(path) + 1) * 2, MAX_PATH * 2));
			return DoMap();
		}
		//映射之后的大小
		LONGLONG getFileSize()
		{
			return m_fSize;
		}
		//映射到进程空间的哪个地址
		PVOID getAddr()
		{
			return m_pFileAddr;
		}

		//获得对齐之后的大小
		//数学模型为：以10为对齐方式，20--->20, 21---->30, 22--->30....29--->30
		size_t getAlignSize(size_t nSize, UINT nAlign)
		{
			if (nAlign == 0)
				return nSize;
			return ((nSize + nAlign - 1) / nAlign * nAlign);
		}

		//• 1.按照系统分页大小对齐SizeOfImage得到ImageSize1
		//• 2.按照OptionalHeader.SectionAlignment对齐ImageSize1得到ImageSize2
		//• 3.遍历Section比较节表结束和ImageSize2取最大值为ImageSize3
		//• 4.此时ImageSize3就是最真实的ImageSize了
		//获得PE文件在内存中真正的大小
		size_t getRealImageSize()
		{
			SYSTEM_INFO si = {};
			GetNativeSystemInfo(&si);
			auto nt_header = ntdll::RtlImageNtHeader(m_pFileAddr);
			auto imageSize = getAlignSize(nt_header->OptionalHeader.SizeOfImage, si.dwPageSize);
			imageSize = getAlignSize(imageSize, nt_header->OptionalHeader.SectionAlignment);

			auto section_header = IMAGE_FIRST_SECTION(nt_header);
			for (int i = 0; i < nt_header->FileHeader.NumberOfSections; ++i)
			{
				//得到该节的大小
				auto nCodeSize = section_header[i].Misc.VirtualSize;
				auto nLoadSize = section_header[i].SizeOfRawData;
				auto nMaxSize = (nLoadSize > nCodeSize) ? (nLoadSize) : (nCodeSize);
				auto nSectionSize = getAlignSize(section_header[i].VirtualAddress + nMaxSize, si.dwPageSize);

				if (imageSize < nSectionSize)
				{
					imageSize = nSectionSize;  //Use the Max;
				}
			}
			return imageSize;
		}

		//可以使用RtlImageDirectoryEntryToData
		//返回虚拟地址：va和大小：size
		PVOID getDataDirectory(UINT index, PULONG size)
		{
			PVOID virtualAddr = NULL;
			PIMAGE_DATA_DIRECTORY dataDir = NULL;
			auto nt_header = ntdll::RtlImageNtHeader(m_pFileAddr);
			auto nt_header64 = reinterpret_cast<PIMAGE_NT_HEADERS64>(nt_header);
			auto nt_header32 = reinterpret_cast<PIMAGE_NT_HEADERS32>(nt_header);
			if (isPE64())	//64位PE
			{
				dataDir = &nt_header64->OptionalHeader.DataDirectory[index];
				if(size)
					*size = (ULONG)nt_header64->OptionalHeader.DataDirectory[index].Size;
			}
			else {	//32位PE
				dataDir = &nt_header32->OptionalHeader.DataDirectory[index];
				if (size)
					*size = (ULONG)nt_header32->OptionalHeader.DataDirectory[index].Size;
			}
			if (dataDir->VirtualAddress && dataDir->Size)
			{
				virtualAddr = reinterpret_cast<PVOID>((PBYTE)m_pFileAddr + dataDir->VirtualAddress);
			}
			return virtualAddr;
		}
		
	private:
		PVOID DoMap()
		{
			LARGE_INTEGER fSize;

			m_hFile = CreateFile(m_path, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ,
				NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
			if (m_hFile == INVALID_HANDLE_VALUE)
				return NULL;

			if (!GetFileSizeEx(m_hFile, &fSize))
			{
				CloseHandle(m_hFile);
				return NULL;
			}
			m_fSize = fSize.QuadPart;

			m_hFilemap = CreateFileMapping(m_hFile, NULL, PAGE_READWRITE, fSize.HighPart, fSize.LowPart, NULL);
			if (m_hFilemap == NULL)
			{
				CloseHandle(m_hFile);
				return NULL;
			}

			m_pFileAddr = MapViewOfFile(m_hFilemap, FILE_MAP_ALL_ACCESS, 0, 0, 0);
			if (m_pFileAddr == NULL)
			{
				CloseHandle(m_hFilemap);
				CloseHandle(m_hFile);
				return NULL;
			}
			return m_pFileAddr;
		}	
		void DoUnMap()
		{
			UnmapViewOfFile(m_pFileAddr);
			CloseHandle(m_hFilemap);
			CloseHandle(m_hFile);
		}

		boolean isPeFile()
		{
			return true;
		}

		boolean isPE64() {
			auto nt_header = ntdll::RtlImageNtHeader(m_pFileAddr);
			if (nt_header->FileHeader.Machine == 0x8664) {
				return true;
			}
			return false;
		}
	private:
		WCHAR m_path[MAX_PATH];
		LONGLONG m_fSize;
		HANDLE m_hFile;
		HANDLE m_hFilemap;
		PVOID  m_pFileAddr;
	};


}