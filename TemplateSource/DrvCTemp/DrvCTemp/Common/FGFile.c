#include "FGFile.h"

static VOID FGInitObjectAttributes(POBJECT_ATTRIBUTES objAttr, PUNICODE_STRING filePath)
{
	InitializeObjectAttributes(objAttr, filePath, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, NULL, 0);
}

NTSTATUS FGCreateFile(PUNICODE_STRING filePath, PHANDLE phFile, PIO_STATUS_BLOCK pios)
{
	OBJECT_ATTRIBUTES objAttr = { 0 };
	NTSTATUS status;

	if (filePath == NULL)
	{
		return STATUS_INVALID_PARAMETER_1;
	}

	InitializeObjectAttributes(&objAttr,
		filePath,
		OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
		NULL, 0);

	status = ZwCreateFile(phFile,
		GENERIC_READ | GENERIC_WRITE,
		&objAttr,
		pios,
		NULL,
		0,
		FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
		FILE_OPEN_IF,
		FILE_NON_DIRECTORY_FILE | FILE_RANDOM_ACCESS | FILE_SYNCHRONOUS_IO_NONALERT,
		NULL, 0);

	return status;
}

NTSTATUS FGCreateDirectory(PUNICODE_STRING directoryPath, PHANDLE phFile, PIO_STATUS_BLOCK pios)
{
	OBJECT_ATTRIBUTES objAttr = { 0 };
	NTSTATUS status;

	if (directoryPath == NULL)
	{
		return STATUS_INVALID_PARAMETER_1;
	}
	InitializeObjectAttributes(&objAttr,
		directoryPath,
		OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
		NULL, 0);

	status = ZwCreateFile(phFile,
		GENERIC_READ | GENERIC_WRITE,
		&objAttr,
		pios,
		0,
		FILE_ATTRIBUTE_DIRECTORY,
		FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
		FILE_OPEN_IF,
		FILE_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT,
		NULL, 0);

	return status;
}

NTSTATUS FGWriteFile(IN PUNICODE_STRING filePath, IN PUNICODE_STRING content, OUT PIO_STATUS_BLOCK pios)
{
	HANDLE hFile = NULL;
	NTSTATUS status;
	LARGE_INTEGER offset = { 0 };

	if (filePath == NULL || content == NULL)
	{
		return STATUS_INVALID_PARAMETER_1 | STATUS_INVALID_PARAMETER_2;
	}

	status = FGCreateFile(filePath, &hFile, pios);
	if (!NT_SUCCESS(status))
	{
		return status;
	}

	status = ZwWriteFile(hFile, NULL, NULL, NULL,
		pios,
		content->Buffer,
		content->Length,
		&offset, NULL);
	ZwClose(hFile);
	return status;
}

NTSTATUS FGReadFile(IN PUNICODE_STRING filePath, OUT PUNICODE_STRING content, OUT PIO_STATUS_BLOCK pios)
{
	NTSTATUS status;
	HANDLE hFile = NULL;
	FILE_STANDARD_INFORMATION si = { 0 };
	LARGE_INTEGER offset = { 0 };

	if (filePath == NULL || content == NULL)
	{
		return STATUS_INVALID_PARAMETER_1;
	}
	status = FGCreateFile(filePath, &hFile, pios);

	if (!NT_SUCCESS(status))
	{
		return status;
	}

	status = ZwQueryInformationFile(hFile, pios, &si, sizeof(FILE_STANDARD_INFORMATION), FileStandardInformation);

	if (!NT_SUCCESS(status))
	{
		return status;
	}

	content->Length = content->MaximumLength = si.EndOfFile.QuadPart;
	content->Buffer = (PWSTR)ExAllocatePoolWithTag(PagedPool, si.EndOfFile.QuadPart, 'ELIF');

	if (content->Buffer == NULL)
	{
		ZwClose(hFile);
		return STATUS_INSUFFICIENT_RESOURCES;
	}

	status = ZwReadFile(hFile, NULL, NULL, NULL,
		pios, content->Buffer, content->Length, &offset, NULL);


	ZwClose(hFile);

	return status;
}

NTSTATUS FGCopyfile(IN PUNICODE_STRING src, IN PUNICODE_STRING dst, PIO_STATUS_BLOCK pios)
{
	NTSTATUS status = 0;
	PVOID buffer = NULL;
	LARGE_INTEGER offset = { 0 };
	HANDLE srcHandle = NULL;
	HANDLE dstHandle = NULL;
	OBJECT_ATTRIBUTES srcObjAttr = { 0 };
	OBJECT_ATTRIBUTES dstObjAttr = { 0 };

	FGInitObjectAttributes(&srcObjAttr, src);
	FGInitObjectAttributes(&dstObjAttr, dst);

	status = ZwCreateFile(&srcHandle, FILE_READ_DATA | FILE_READ_ATTRIBUTES, &srcObjAttr, pios, NULL,
		FILE_ATTRIBUTE_NORMAL, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
		FILE_OPEN, FILE_SYNCHRONOUS_IO_NONALERT, NULL, 0);

	if (!NT_SUCCESS(status))
	{
		return status;
	}

	status = ZwCreateFile(&dstHandle, GENERIC_WRITE, &dstObjAttr, pios, NULL,
		FILE_ATTRIBUTE_NORMAL, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
		FILE_OPEN_IF, FILE_SYNCHRONOUS_IO_NONALERT, NULL, 0);

	if (!NT_SUCCESS(status))
	{
		ZwClose(srcHandle);
		return status;
	}

	buffer = ExAllocatePoolWithTag(PagedPool, 4 * 1024, 'ELIF');

	if (buffer == NULL)
	{
		ZwClose(srcHandle);
		ZwClose(dstHandle);
		return STATUS_INSUFFICIENT_RESOURCES;
	}

	while (1)
	{
		int length = 4 * 1024;
		status = ZwReadFile(srcHandle, NULL, NULL, NULL, pios, buffer, length, &offset, NULL);
		if (!NT_SUCCESS(status))
		{
			if (status == STATUS_END_OF_FILE)
			{
				status = STATUS_SUCCESS;
			}
			break;
		}

		length = (ULONG)pios->Information;

		status = ZwWriteFile(dstHandle, NULL, NULL, NULL, pios, buffer, length, &offset, NULL);

		if (!NT_SUCCESS(status))
		{
			break;
		}

		offset.QuadPart += length;
	}

	ExFreePool(buffer);
	ZwClose(srcHandle);
	ZwClose(dstHandle);

	return STATUS_SUCCESS;
}

NTSTATUS FGMovefile(IN PUNICODE_STRING src, IN PUNICODE_STRING dst, OUT PIO_STATUS_BLOCK pios)
{
	
	NTSTATUS status = 0;
	status = FGCopyfile(src, dst, pios);

	if (NT_SUCCESS(status))
	{
		status = FGDeleteFile(src, pios);
	}

	return status;
}

NTSTATUS FGDeleteFile(IN PUNICODE_STRING filePath, PIO_STATUS_BLOCK pios)
{
	NTSTATUS status = 0;
	HANDLE fHandle = NULL;
	OBJECT_ATTRIBUTES objAttr = { 0 };
	FILE_DISPOSITION_INFORMATION disInfo = { 0 };

	FGInitObjectAttributes(&objAttr, filePath);

	status = ZwCreateFile(
		&fHandle,
		GENERIC_ALL,
		&objAttr,
		pios,
		NULL,
		FILE_ATTRIBUTE_NORMAL,
		FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
		FILE_OPEN,
		FILE_SYNCHRONOUS_IO_NONALERT | FILE_DELETE_ON_CLOSE,
		NULL,
		0);

	if (!NT_SUCCESS(status))
	{
		if (status == STATUS_ACCESS_DENIED)
		{
			status = ZwCreateFile(
				&fHandle,
				SYNCHRONIZE | FILE_READ_ATTRIBUTES | FILE_WRITE_ATTRIBUTES,
				&objAttr,
				pios,
				NULL,
				FILE_ATTRIBUTE_NORMAL,
				FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
				FILE_OPEN,
				FILE_SYNCHRONOUS_IO_NONALERT,
				NULL,
				0);

			if (NT_SUCCESS(status))
			{
				FILE_BASIC_INFORMATION basicInfo = { 0 };
				status = ZwQueryInformationFile(fHandle, pios, &basicInfo, sizeof(basicInfo), FileBasicInformation);
				if (!NT_SUCCESS(status))
				{
					DbgPrint("ZwQueryInformationFile(%wZ) failed(%x)\n", filePath, status);
				}

				basicInfo.FileAttributes = FILE_ATTRIBUTE_NORMAL;
				status = ZwSetInformationFile(fHandle, pios, &basicInfo, sizeof(basicInfo), FileBasicInformation);
				if (!NT_SUCCESS(status))
				{
					DbgPrint("ZwSetInformationFile(%wZ) failed(%x)\n", filePath, status);
				}
				ZwClose(fHandle);

				status = ZwCreateFile(
					&fHandle,
					SYNCHRONIZE | FILE_WRITE_DATA | DELETE,
					&objAttr,
					pios,
					NULL,
					FILE_ATTRIBUTE_NORMAL,
					FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
					FILE_OPEN,
					FILE_SYNCHRONOUS_IO_NONALERT | FILE_DELETE_ON_CLOSE,
					NULL,
					0);
			}
		}

		if (!NT_SUCCESS(status))
		{
			return status;
		}
	}

	disInfo.DeleteFile = TRUE;
	status = ZwSetInformationFile(fHandle, pios, &disInfo, sizeof(disInfo), FileDispositionInformation);
	if (!NT_SUCCESS(status))
	{
		ZwClose(fHandle);
		DbgPrint("ZwSetInformationFile failed (%x)\n ", status);
	}
	ZwClose(fHandle);
	return status;
}