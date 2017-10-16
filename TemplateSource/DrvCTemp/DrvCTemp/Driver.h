#pragma once
#include "stdafx.h"

//为避免混淆这两个要一致
#define  DEVICE_NAME L"\\device\\PopWinDrv"
#define  LINK_NAME   L"\\dosdevices\\PopWinDrv"

#define IOCTRL_BASE 0X8000
#define FGIOCTRL_CODE(i) \
	CTL_CODE(FILE_DEVICE_UNKNOWN, IOCTRL_BASE + i, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_SEND_RESULT_TO_R0 FGIOCTRL_CODE(0)
#define IOCTL_XXX_ATTACK        FGIOCTRL_CODE(1)

/*卸载函数*/
VOID DriverUnload(PDRIVER_OBJECT pDriverObject);
NTSTATUS CreateDevice(IN PDRIVER_OBJECT pDriverObject);
/*通用分发函数*/
NTSTATUS DispatchCommon(PDEVICE_OBJECT pDevObj, PIRP pIrp);
/*创建分发函数*/
NTSTATUS DispatchCreate(PDEVICE_OBJECT pDevObj, PIRP pIrp);
/*读分发函数*/
NTSTATUS DispatchRead(PDEVICE_OBJECT pDevObj, PIRP pIrp);
/*写分发函数*/
NTSTATUS DispatchWrite(PDEVICE_OBJECT pDevObj, PIRP pIrp);
/*控制分发函数*/
NTSTATUS DispatchIoCtl(PDEVICE_OBJECT pDevObj, PIRP pIrp);
/*清除分发函数*/
NTSTATUS DispatchClean(PDEVICE_OBJECT pDevObj, PIRP pIrp);
/*关闭分发函数*/
NTSTATUS DispatchClose(PDEVICE_OBJECT pDevObj, PIRP pIrp);