#include "stdafx.h"

VOID DriverUnload(__in DRIVER_OBJECT *driverObject)
{
	UNREFERENCED_PARAMETER(driverObject);
	LOG_DEBUG("unload\r\n");
	return;
}
_Use_decl_annotations_
EXTERN_C
NTSTATUS
DriverMain(
	__in DRIVER_OBJECT* driverObject,
	__in UNICODE_STRING* registryPath
)
{
	driverObject->DriverUnload = DriverUnload;
	UNREFERENCED_PARAMETER(driverObject);
	UNREFERENCED_PARAMETER(registryPath);
	
	LOG_DEBUG("hello world\r\n");
	return STATUS_SUCCESS;
}