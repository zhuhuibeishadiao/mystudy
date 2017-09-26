#include "stdafx.h"

extern "C"
{
	DRIVER_INITIALIZE DriverEntry;
	DRIVER_INITIALIZE DriverMain;
	DRIVER_UNLOAD UnloadDriver;
};

PDRIVER_UNLOAD Old_Unload = nullptr;

PDRIVER_OBJECT g_pDriverObject = nullptr;
PVOID g_pDrvImageBase = nullptr;
SIZE_T g_DrvImageSize = 0;
wchar_t g_DriverRegistryKey[MAX_PATH] = {};
PLIST_ENTRY PsLoadedModuleList = nullptr;

_Use_decl_annotations_
EXTERN_C
VOID UnloadDriver(
	_In_ DRIVER_OBJECT *DriverObject
)
{
	if (Old_Unload)
	{
		Old_Unload(DriverObject);
	}
	//沉睡三秒关闭宇宙
	ddk::util::sleep(ddk::util::seconds(3));
	cc_doexit(0,0,0);
}
_Use_decl_annotations_
EXTERN_C
NTSTATUS
DriverEntry(
	__in DRIVER_OBJECT* driverObject,
	__in UNICODE_STRING* registryPath
)
{
	NTSTATUS ns = STATUS_UNSUCCESSFUL;
	//先初始化版本號信息
	ddk::util::init_version();
	if (OsIndex==OsIndex_UNK)
	{
		LOG_DEBUG("unknown system version\r\n");
		return STATUS_UNSUCCESSFUL;
	}
	//CRT init
	cc_init(0);
	//初始化mem工具函數
	ddk::mem_util::init_mem_util();

	if (registryPath)
	{
		RtlSecureZeroMemory(g_DriverRegistryKey, sizeof(g_DriverRegistryKey));
		RtlStringCchPrintfW(g_DriverRegistryKey,
			RTL_NUMBER_OF(g_DriverRegistryKey), L"%wZ",
			registryPath);
		if (driverObject)
		{
			*(PULONG)((PCHAR)driverObject->DriverSection + 13 * sizeof(void*)) |= 0x20;
			g_pDriverObject = driverObject;
			auto entry = reinterpret_cast<KLDR_DATA_TABLE_ENTRY *>(g_pDriverObject->DriverSection);
			g_pDrvImageBase = entry->DllBase;
			g_DrvImageSize = entry->SizeOfImage;
			PsLoadedModuleList = entry->InLoadOrderLinks.Flink;
			LOG_DEBUG("DriverImageBase= %p ImageSize=%x\r\n", g_pDrvImageBase, g_DrvImageSize);
		}
	}

	ns = DriverMain(driverObject, registryPath);
	if (!NT_SUCCESS(ns))
	{
		UnloadDriver(driverObject);
	}
	else
	{
		Old_Unload = driverObject->DriverUnload;
		if(Old_Unload)
			driverObject->DriverUnload = UnloadDriver;
	}
	
	return ns;
}