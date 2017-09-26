#include "stdafx.h"

ddk::nt_reg::nt_reg()
{
	h_key = nullptr;
	key_ref = 0;
}


ddk::nt_reg::~nt_reg()
{
	if (key_ref == 0 && h_key)
	{
		ZwClose(h_key);
	}
	LOG_DEBUG("release reg\r\n");
}
ddk::nt_reg::nt_reg(std::wstring strKey)
{
	h_key = nullptr;
	key_ref = 0;
	open(strKey);
}

ddk::nt_reg::nt_reg(DDK_HKEY key, std::wstring strKey)
{
	h_key = nullptr;
	key_ref = 0;
	open(key, strKey);
}
//ddk::nt_reg::nt_reg(nt_reg &key)
//{
//	h_key = nullptr;
//	key_ref = 0;
//	h_key = key.get_handle();
//}

//const HANDLE ddk::nt_reg::get_handle()
//{
//	InterlockedIncrement(&key_ref);
//	return h_key;
//}


bool ddk::nt_reg::open(std::wstring strKey)
{
	key_string = strKey;
	UNICODE_STRING usKeyName = { 0 };
	OBJECT_ATTRIBUTES oa = { 0 };
	RtlInitUnicodeString(&usKeyName, strKey.c_str());
	InitializeObjectAttributes(&oa,
		&usKeyName,
		OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
		NULL,
		NULL);
	ULONG ulResult = 0;
	auto ns = ZwCreateKey(&h_key,
		KEY_ALL_ACCESS,
		&oa,
		0,
		NULL,
		REG_OPTION_NON_VOLATILE,//重启后仍然在，
								// 如果使用REG_OPTION_VOLATILE，重启后，注册表就消失了。
		&ulResult //这里保存键状态：
				  // REG_CREATED_NEW_KEY 新建的键！
				  // REG_OPENED_EXISTING_KEY 已经存在的键
	);
	if (ns == STATUS_SUCCESS)
	{
		LOG_DEBUG("open key ok\r\n");
		return true;
	}
	if (!NT_SUCCESS(ns))
	{
		ddk::status::LogStatus(ns);
		return false;
	}
	return false;
}


bool ddk::nt_reg::open(DDK_HKEY key, std::wstring strKey)
{
	std::wstring full_key = std::wstring(L"");
	switch (key)
	{
	case ddk::nt_reg::HKEY_CLASSES_ROOT:
		full_key += L"\\Registry\\Machine\\Software\\CLASSES";
		break;
	case ddk::nt_reg::HKEY_CURRENT_USER:
		//这个很复杂
		full_key += get_current_user();
		break;
	case ddk::nt_reg::HKEY_LOCAL_MACHINE:
		full_key += L"\\Registry\\Machine";
		break;
	case ddk::nt_reg::HKEY_USERS:
		full_key += L"\\Registry\\User";
		break;
	case ddk::nt_reg::HKEY_CURRENT_CONFIG:
		full_key += L"\\Registry\\Machine\\System\\CurrentControlSet\\Hardware Profiles\\Current";
		break;
	default:
		return false;
		break;
	}
	full_key += L"\\";
	full_key += strKey;
	return open(full_key);
}


std::wstring ddk::nt_reg::get_current_user()
{
	HANDLE hRegister;
	OBJECT_ATTRIBUTES ObjectAttributes;
	ULONG ulSize;
	UNICODE_STRING uniKeyName;
	WCHAR CurrentUserbuf[256];
	WCHAR ProfileListbuf[256];
	UNICODE_STRING RegCurrentUser, RegUser;
	UNICODE_STRING RegProfileList, RegProf;
	RTL_QUERY_REGISTRY_TABLE paramTable[2];
	ULONG udefaultData = 0;
	ULONG uQueryValue;

	RtlZeroMemory(paramTable, sizeof(paramTable));

	paramTable[0].Flags = RTL_QUERY_REGISTRY_DIRECT;
	paramTable[0].Name = L"RefCount";
	paramTable[0].EntryContext = &uQueryValue;
	paramTable[0].DefaultType = REG_DWORD;
	paramTable[0].DefaultData = &udefaultData;
	paramTable[0].DefaultLength = sizeof(ULONG);


	RtlInitUnicodeString(&RegProf, L"\\Registry\\Machine\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\ProfileList\\");
	RtlInitUnicodeString(&RegUser, L"\\Registry\\User\\");

	RtlInitEmptyUnicodeString(&RegCurrentUser, CurrentUserbuf, 256 * sizeof(WCHAR));
	RtlInitEmptyUnicodeString(&RegProfileList, ProfileListbuf, 256 * sizeof(WCHAR));

	RtlCopyUnicodeString(&RegCurrentUser, &RegUser);
	RtlCopyUnicodeString(&RegProfileList, &RegProf);

	InitializeObjectAttributes(&ObjectAttributes, &RegProf, OBJ_CASE_INSENSITIVE, NULL, NULL);
	do
	{

		auto ns = ZwOpenKey(&hRegister, KEY_ALL_ACCESS, &ObjectAttributes);
		if (!NT_SUCCESS(ns))
		{
			break;
		}
		auto hkey_exit = std::experimental::make_scope_exit([&]() {ZwClose(hRegister); });
		ns = ZwQueryKey(hRegister, KeyFullInformation, NULL, 0, &ulSize);
		if (ns != STATUS_BUFFER_TOO_SMALL&&ns != STATUS_BUFFER_OVERFLOW)
		{
			break;
		}
		auto pfi = (PKEY_FULL_INFORMATION)malloc(ulSize);
		auto pfi_exit = std::experimental::make_scope_exit([&]() {free(pfi); });
		ns = ZwQueryKey(hRegister, KeyFullInformation, pfi, ulSize, &ulSize);
		if (!NT_SUCCESS(ns))
		{
			break;
		}

		for (auto i = ULONG(0); i < pfi->SubKeys; i++)
		{
			ns = ZwEnumerateKey(hRegister,
				i,
				KeyBasicInformation,
				NULL,
				0,
				&ulSize);
			if (ns != STATUS_BUFFER_TOO_SMALL&&ns != STATUS_BUFFER_OVERFLOW)
			{
				break;
			}
			auto pbi = (PKEY_BASIC_INFORMATION)malloc(ulSize);
			auto pbi_exit = std::experimental::make_scope_exit([&]() {free(pbi); });
			ns = ZwEnumerateKey(hRegister,
				i,
				KeyBasicInformation,
				pbi,
				ulSize,
				&ulSize);
			if (!NT_SUCCESS(ns))
			{
				break;
			}

			uniKeyName.Length = uniKeyName.MaximumLength = (USHORT)pbi->NameLength;
			uniKeyName.Buffer = pbi->Name;

			if (pbi->NameLength > 20)
			{
				RtlAppendUnicodeStringToString(&RegCurrentUser, &uniKeyName);
				RtlAppendUnicodeStringToString(&RegProfileList, &uniKeyName);
				RtlQueryRegistryValues(RTL_REGISTRY_ABSOLUTE, RegProfileList.Buffer, paramTable, NULL, NULL);
				if (uQueryValue > 0)
				{
					LOG_DEBUG("HKET_CURRENT_USER: %wZ\n", &RegCurrentUser);
					return std::wstring(CurrentUserbuf);
				}
			}
			RtlCopyUnicodeString(&RegCurrentUser, &RegUser);
			RtlCopyUnicodeString(&RegProfileList, &RegProf);
		}
	} while (0);
	return std::wstring(L"");
}


bool ddk::nt_reg::del_key()
{
	if (h_key)
	{
		auto ns = ZwDeleteKey(h_key);
		if (ns == STATUS_SUCCESS)
		{
			ZwClose(h_key);
			h_key = nullptr;
			return true;
		}
	}
	return false;
}


bool ddk::nt_reg::del_value(std::wstring val_name)
{
	if (h_key)
	{
		UNICODE_STRING usValueName;
		RtlInitUnicodeString(&usValueName, val_name.c_str());
		auto ns = ZwDeleteValueKey(h_key, &usValueName);
		if (ns == STATUS_SUCCESS)
		{
			return true;
		}
	}
	return false;
}


bool ddk::nt_reg::set_value(std::wstring value_name, ULONG val_type, PVOID value_data, size_t data_size)
{
	if (h_key)
	{
		UNICODE_STRING usValueName = { 0 };
		RtlInitUnicodeString(&usValueName, value_name.c_str());
		auto ns = ZwSetValueKey(h_key,
			&usValueName,
			0,
			val_type,
			value_data,
			(ULONG)data_size);
		if (NT_SUCCESS(ns))
		{
			return true;
		}
	}
	return false;
}


bool ddk::nt_reg::get_value(std::wstring value_name, ULONG val_type, PVOID value_buffer, size_t & buffer_size)
{
	if (h_key)
	{
		ULONG ulSize = 0;
		UNICODE_STRING usValueName = { 0 };
		RtlInitUnicodeString(&usValueName, value_name.c_str());
		auto ns = ZwQueryValueKey(h_key,
			&usValueName,
			KeyValuePartialInformation,
			NULL,
			0,
			&ulSize);
		if (ns == STATUS_BUFFER_OVERFLOW || ns == STATUS_BUFFER_TOO_SMALL)
		{
			auto pkvpi = (PKEY_VALUE_PARTIAL_INFORMATION)malloc(ulSize);//CHK模式下是不能使用ExAllocatePool的
			if (pkvpi)
			{
				RtlZeroMemory(pkvpi, ulSize);
				ns = ZwQueryValueKey(h_key,
					&usValueName,
					KeyValuePartialInformation,
					pkvpi,
					ulSize,
					&ulSize);
				if (NT_SUCCESS(ns))
				{
					if (pkvpi->Type != val_type)
					{
						return false;
					}
					if (pkvpi->DataLength <= buffer_size)
					{
						buffer_size = pkvpi->DataLength;
						if (value_buffer)
							RtlCopyMemory(value_buffer, pkvpi->Data, pkvpi->DataLength);
						return true;
					}
					buffer_size = pkvpi->DataLength;
				}
				free(pkvpi);
			}
		}
	}
	return false;
}


void ddk::nt_reg::close()
{
	if (h_key && key_ref == 0)
	{
		ZwClose(h_key);
	}
	if (key_ref > 0)
	{
		InterlockedDecrement(&key_ref);
	}
}


ddk::nt_reg ddk::nt_reg::create_key(std::wstring key_name)
{
	auto new_key = key_string + L"\\" + key_name;
	auto ret = ddk::nt_reg(new_key);
	return ret;
}