#include "FGReg.h"

static VOID FGInitObjectAttributes(POBJECT_ATTRIBUTES pObjAttr, PUNICODE_STRING path)
{
	InitializeObjectAttributes(pObjAttr, path, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, NULL, 0);
}

NTSTATUS FGCreateKey(PUNICODE_STRING keyPath)
{
	NTSTATUS status = 0;
	ULONG uResults = 0;
	OBJECT_ATTRIBUTES objAttr = { 0 };
	HANDLE hKey = NULL;

	FGInitObjectAttributes(&objAttr, keyPath);
	status = ZwCreateKey(&hKey, KEY_ALL_ACCESS, &objAttr, 0, NULL, REG_OPTION_NON_VOLATILE, &uResults);

	if (!NT_SUCCESS(status))
	{
		ZwClose(hKey);
		return status;
	}
	
	ZwClose(hKey);
	return status;
}

NTSTATUS FGOpenKey(IN PUNICODE_STRING regPath, OUT PHANDLE phReg)
{
	NTSTATUS status = 0;
	OBJECT_ATTRIBUTES objAttr = { 0 };

	FGInitObjectAttributes(&objAttr, regPath);
	status = ZwOpenKey(phReg, KEY_ALL_ACCESS, &objAttr);

	if (NT_SUCCESS(status))
	{
		return status;
	}

	return status;
}

NTSTATUS FGDeleteKey(IN PUNICODE_STRING keyPath)
{
	NTSTATUS status = 0;
	HANDLE hKey = NULL;
	status = FGOpenKey(keyPath, &hKey);

	if (!NT_SUCCESS(status))
	{
		return status;
	}

	status = ZwDeleteKey(hKey);
	ZwClose(hKey);
	return status;
}

NTSTATUS FGSetValueKey(IN PUNICODE_STRING keyPath, IN ULONG type, IN PUNICODE_STRING valueName, IN PUNICODE_STRING value)
{
	NTSTATUS status = 0;
	HANDLE hKey = NULL;
	status = FGOpenKey(keyPath, &hKey);

	if (!NT_SUCCESS(status))
	{
		return status;
	}

	switch (type)
	{
	case REG_DWORD:
		{
			ULONG uValue = 0;
			RtlUnicodeStringToInteger(value, 10, &uValue);
			status = ZwSetValueKey(hKey, valueName, 0, type, &uValue, sizeof(ULONG));
			if (!NT_SUCCESS(status))
			{
				ZwClose(hKey);
				return status;
			}
		}
		break;

	case REG_BINARY:
	case REG_SZ:
	case REG_EXPAND_SZ:
	case REG_MULTI_SZ:
		{
			status = ZwSetValueKey(hKey, valueName, 0, type, value->Buffer, value->Length);
			if (!NT_SUCCESS(status))
			{
				ZwClose(hKey);
				return status;
			}
		}
		break;
	default:
		break;
	}

	ZwClose(hKey);
	return status;
}

NTSTATUS FGdeleteValueKey(IN PUNICODE_STRING keyPath, PUNICODE_STRING valuename)
{
	NTSTATUS status = 0;
	HANDLE hKey = NULL;

	status = FGOpenKey(keyPath, &hKey);
	if (!NT_SUCCESS(STATUS_SUCCESS))
	{
		return status;
	}

	status = ZwDeleteValueKey(hKey, valuename);
	ZwClose(hKey);
	return STATUS_SUCCESS;
}

NTSTATUS FGQueryDword(IN PUNICODE_STRING keyPath, IN PUNICODE_STRING valuename, OUT PULONG value)
{
	NTSTATUS status = 0;
	ULONG ulSize = 0;
	HANDLE hKey = NULL;
	PKEY_VALUE_PARTIAL_INFORMATION pvpi = NULL;
	
	status = FGOpenKey(keyPath, &hKey);
	if (!NT_SUCCESS(status))
	{
		return status;
	}

	status = ZwQueryValueKey(hKey, valuename, KeyValuePartialInformation, NULL, 0, &ulSize);
	if (status != STATUS_BUFFER_OVERFLOW &&
		status != STATUS_BUFFER_TOO_SMALL)
	{
		ZwClose(hKey);
		return status;
	}

	pvpi = (PKEY_VALUE_PARTIAL_INFORMATION)ExAllocatePoolWithTag(PagedPool, ulSize, 'DOWD');
	if (pvpi == NULL)
	{
		return STATUS_INSUFFICIENT_RESOURCES;
	}

	status = ZwQueryValueKey(hKey, valuename, KeyValuePartialInformation, pvpi, ulSize, &ulSize);
	
	if (!NT_SUCCESS(status))
	{
		ExFreePool(pvpi);
		ZwClose(hKey);
		return status;
	}

	if (pvpi->DataLength != sizeof(ULONG))
	{

		return STATUS_INVALID_PARAMETER;
	}
	*value = *((PULONG)pvpi->Data);

	return status;
}

NTSTATUS FGQuerySZ(IN PUNICODE_STRING keyPath, IN PUNICODE_STRING valuename, OUT PUNICODE_STRING value)
{
	NTSTATUS status = 0;
	ULONG ulSize = 0;
	HANDLE hKey = NULL;
	PKEY_VALUE_PARTIAL_INFORMATION pvpi = NULL;

	status = FGOpenKey(keyPath, &hKey);

	if (!NT_SUCCESS(status))
	{
		return status;
	}

	status = ZwQueryValueKey(hKey, valuename, KeyValuePartialInformation, NULL, 0, &ulSize);
	if (status == STATUS_OBJECT_NAME_NOT_FOUND || ulSize == 0)
	{
		ZwClose(hKey);
		return status;
	}

	pvpi = (PKEY_VALUE_PARTIAL_INFORMATION)ExAllocatePoolWithTag(PagedPool, ulSize, 'SGER');
	if (pvpi == NULL)
	{
		return STATUS_INSUFFICIENT_RESOURCES;
	}

	status = ZwQueryValueKey(hKey, valuename, KeyValuePartialInformation, pvpi, ulSize, &ulSize);

	if (!NT_SUCCESS(status))
	{
		ExFreePool(pvpi);
		ZwClose(hKey);
		return status;
	}

	value->Length = value->MaximumLength = pvpi->DataLength;
	value->Buffer = pvpi->Data;

	RtlCopyMemory(value->Buffer, pvpi->Data, pvpi->DataLength);
	
	ExFreePool(pvpi);
	ZwClose(hKey);
	return status;
}

