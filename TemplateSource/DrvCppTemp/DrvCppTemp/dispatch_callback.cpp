#include "stdafx.h"
#include "ioctrl.h"

extern ddk::nt_device device;	//参数来自于Main.cpp
NTSTATUS Ioctrl_Handle1(PVOID InputBuffer,
	ULONG InputBufferSize,
	PVOID OutputBuffer,
	ULONG OutputBufferSize,
	ULONG_PTR *ReturnSize)
{
	LOG_DEBUG("控制码800\r\n");
	*ReturnSize = 0;
	return STATUS_SUCCESS;
}

NTSTATUS Ioctrl_Handle2(PVOID InputBuffer,
	ULONG InputBufferSize,
	PVOID OutputBuffer,
	ULONG OutputBufferSize,
	ULONG_PTR *ReturnSize, 
	int xxx)
{
	DbgPrint("hello world %x\r\n", xxx);
	*ReturnSize = 0;
	return STATUS_SUCCESS;
}


void setDpcFun()
{

	//ioctl
	device.set_ioctrl_callback(FG_IOCTL_HELLO, Ioctrl_Handle1);
	auto bind_fun = std::bind(&Ioctrl_Handle2, std::placeholders::_1, std::placeholders::_2,
		std::placeholders::_3, std::placeholders::_4, std::placeholders::_5, 0x123456);
	device.set_ioctrl_callback(FG_IOCTL_HELLO2, bind_fun);	//可以使用bind
}