#pragma once
namespace ddk::string
{
	inline wchar_t * a2w(const char * ansi, SIZE_T szStr)
	{
		wchar_t * buffer = NULL;
		SIZE_T i;

		if (ansi && szStr)
		{
			buffer = (wchar_t *)malloc((szStr + 1) * sizeof(wchar_t));
			if (buffer)
			{
				for (i = 0; i < szStr; i++)
					buffer[i] = ansi[i];
			}
		}
		return buffer;
	}
	inline wchar_t *A2WEX(const char *ansi)
	{
		wchar_t *buffer = nullptr;
		ANSI_STRING asName;
		UNICODE_STRING usName;
		NTSTATUS ns;
		RtlInitAnsiString(&asName, ansi);
		ns = RtlAnsiStringToUnicodeString(&usName, &asName, TRUE);
		if (NT_SUCCESS(ns))
		{
			buffer = (wchar_t *)malloc(usName.MaximumLength);
			if (buffer)
			{
				RtlZeroMemory(buffer, usName.MaximumLength);
				RtlCopyMemory(buffer, usName.Buffer, usName.Length);
			}
			RtlFreeUnicodeString(&usName);
		}
		return buffer;
	}
	inline wchar_t * a2w(const char * ansi)
	{
		wchar_t * buffer = NULL;
		if (ansi)
			buffer = a2w(ansi, strlen(ansi));
		return buffer;
	}
	inline char *W2AEX(const wchar_t *wide)
	{
		char *buffer = nullptr;
		UNICODE_STRING usName = {};
		ANSI_STRING asName;
		RtlInitUnicodeString(&usName, wide);
		auto ns = RtlUnicodeStringToAnsiString(&asName, &usName, TRUE);
		if (NT_SUCCESS(ns))
		{
			buffer = (char *)malloc(asName.MaximumLength);
			if (buffer)
			{
				RtlZeroMemory(buffer, asName.MaximumLength);
				RtlCopyMemory(buffer, asName.Buffer, asName.Length);
			}
			RtlFreeAnsiString(&asName);
		}
		return buffer;
	}
};