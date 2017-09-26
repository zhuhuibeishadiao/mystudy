#include "stdafx.h"

namespace ddk::util
{
	ULONG LdrGetImageSize(PVOID imageBase)
	{
		auto size = 0UL;
		do 
		{
			__try
			{
				auto dosHeader = (PIMAGE_DOS_HEADER)imageBase;
				if (dosHeader->e_magic != IMAGE_DOS_SIGNATURE)
				{
					break;
				}
				auto ntheader = (PIMAGE_NT_HEADERS)((PUCHAR)imageBase + dosHeader->e_lfanew);
				if (ntheader->Signature!=IMAGE_NT_SIGNATURE)
				{
					break;
				}
				if (ntheader)
				{
				}
				size = (ntheader->OptionalHeader.SizeOfCode);
			}
			__except (1)
			{
				return 0UL;
			}
		} while (0);

		return size;
	}
	//////////////////////////////////////////////////////////////////////////
	PIMAGE_RESOURCE_DATA_ENTRY FindResource(PVOID Module, PCWSTR Name, PCWSTR Type)
	{
		NTSTATUS                    Status;
		ULONG_PTR                   ResourceIdPath[3];
		PIMAGE_RESOURCE_DATA_ENTRY  ResourceDataEntry;

		ResourceIdPath[0] = (ULONG_PTR)Type;
		ResourceIdPath[1] = (ULONG_PTR)Name;
		ResourceIdPath[2] = 0;

		auto _LdrFindResource_U = ddk::util::nt_syscall::getInstance().get<decltype(&LdrFindResource_U)>("LdrFindResource_U");
		if (!_LdrFindResource_U)
		{
			return nullptr;
		}
		Status = _LdrFindResource_U(Module, ResourceIdPath, ARRAYSIZE(ResourceIdPath), &ResourceDataEntry);

		if (NT_SUCCESS(Status))
			return ResourceDataEntry;
		return nullptr;
	}
	PVOID LoadResource(PVOID Module, PIMAGE_RESOURCE_DATA_ENTRY ResourceDataEntry, PULONG Size)
	{
		PVOID       Address;
		NTSTATUS    Status;
		auto _LdrAccessResource = ddk::util::nt_syscall::getInstance().get<decltype(&LdrAccessResource)>("LdrAccessResource");
		if (!_LdrAccessResource)
		{
			return nullptr;
		}
		Status = _LdrAccessResource(Module, ResourceDataEntry, &Address, Size);
		if (NT_SUCCESS(Status))
			return Address;
		return nullptr;
	}
};