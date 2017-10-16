#include "stdafx.h"
#include "ioctrl.h"

extern void setDpcFun();

ddk::nt_device device;
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
	UNREFERENCED_PARAMETER(driverObject);
	UNREFERENCED_PARAMETER(registryPath);
	driverObject->DriverUnload = DriverUnload;
	  
	//…Ë÷√dispatch∫Ø ˝
	setDpcFun();
	

	if (device.create_device(DEVICE_NAME, LINK_NAME, true))
		return STATUS_SUCCESS;
	return STATUS_UNSUCCESSFUL;
}