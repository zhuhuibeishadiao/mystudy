#pragma once
//这个头文件直接拿到应用层
#ifndef _NTDDK_
#include <winioctl.h>
#endif

//为避免混淆这两个要一致
#define  DEVICE_NAME L"\\Device\\FGDrv"
#define  LINK_NAME   L"\\Dosdevices\\FGDrv"

//使用时第二个参数按顺序增加，大小在0x0800-0x0FFF
static const auto DEVICE_CODE = 0x8000ul;
static const auto FG_IOCTL_HELLO = CTL_CODE(DEVICE_CODE, 0x0800, METHOD_BUFFERED, FILE_ANY_ACCESS);
static const auto FG_IOCTL_HELLO2 = CTL_CODE(DEVICE_CODE, 0x0801, METHOD_BUFFERED, FILE_ANY_ACCESS);

