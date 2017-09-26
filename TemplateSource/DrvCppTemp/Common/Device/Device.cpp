#include "stdafx.h"

namespace ddk
{
	static const auto default_device_code = 0x8000ul;
};

ddk::nt_device::nt_device():device_object(nullptr)
{
	driver_object = nullptr;
	Asyn_able = false;
	dwDeviceCode = ddk::default_device_code;
	map_ioctrl.clear();
	map_irp_routine.clear();
	set_irp_callback(IRP_MJ_CREATE, ddk::nt_device::default_irp_routine);
	set_irp_callback(IRP_MJ_CLOSE, ddk::nt_device::default_irp_routine);
	set_irp_callback(IRP_MJ_READ, ddk::nt_device::default_irp_routine);
	set_irp_callback(IRP_MJ_WRITE, ddk::nt_device::default_irp_routine);

	driver_object = ddk::nt_drivers::getInstance().get_new_driver();
}


ddk::nt_device::~nt_device()
{
	if (device_object!=nullptr)
	{
		DrvTerminater();
	}
	if (driver_object)
	{
		ddk::nt_drivers::getInstance().del_driver_obj(driver_object);
	}
}


void ddk::nt_device::DrvTerminater()
{
	auto DeviceExtension = (PDEVICE_EXTENSION)device_object->DeviceExtension;

	if (DeviceExtension->ThreadObject)
	{
		DeviceExtension->bTerminateThread = TRUE;
		KeSetEvent(&DeviceExtension->RequestEvent, 0, FALSE);
		KeWaitForSingleObject(DeviceExtension->ThreadObject, Executive, KernelMode, FALSE, NULL);
		ObDereferenceObject(DeviceExtension->ThreadObject);
	}

	if (DeviceExtension->SecurityClientCtx)
	{
		SeDeleteClientSecurity(DeviceExtension->SecurityClientCtx);
		free(DeviceExtension->SecurityClientCtx);
	}

	IoDeleteSymbolicLink(&nsDosName);
	IoDeleteDevice(device_object);
}


void ddk::nt_device::set_device_code(DWORD dwCode)
{
	dwDeviceCode = dwCode;
}


void ddk::nt_device::set_ioctrl_callback(DWORD code, callback_ioctrl callback)
{
	map_ioctrl[code] = callback;
}


void ddk::nt_device::set_irp_callback(int irp, callback_irp callback)
{
	map_irp_routine[irp] = callback;
}


bool ddk::nt_device::create_device(LPCWSTR device_name, LPCWSTR dos_name, bool b_asyn)
{
	if (device_object)
	{
		return false;
	}
	if (!driver_object)
	{
		return false;
	}
	Asyn_able = b_asyn;
	auto status = AuxKlibInitialize();
	if (!NT_SUCCESS(status)) {
		return false;
	}
	RtlInitUnicodeString(&nsDosName, dos_name);
	RtlInitUnicodeString(&nsDeviceName, device_name);
	status = IoCreateDeviceSecure(driver_object,
		sizeof(DEVICE_EXTENSION),
		&nsDeviceName,
		dwDeviceCode,
		FILE_DEVICE_SECURE_OPEN,
		FALSE,
		&SDDL_DEVOBJ_SYS_ALL_ADM_ALL, nullptr,
		&device_object);
	if (!NT_SUCCESS(status))
	{
		return false;
	}
	auto DeviceExtension = (PDEVICE_EXTENSION)device_object->DeviceExtension;

	device_object->Flags &=	~DO_DEVICE_INITIALIZING;//不在DriverEntry流程里，也就是不收IO管理器控制的DeviceObject创建需要自己清除init标记

	device_object->Flags |= DO_DIRECT_IO;//I/O时使用MmGetSystemAddressForMdlSafe得到buffer
	RtlZeroMemory(DeviceExtension, sizeof(DEVICE_EXTENSION));

	DeviceExtension->Tag = 'obj1';
	DeviceExtension->bTerminateThread = FALSE;
	InitializeListHead(&DeviceExtension->ListHead);
	/*KeInitializeSpinLock(&DeviceExtension->ListLock);*/
	KeInitializeEvent(&DeviceExtension->RequestEvent, SynchronizationEvent, FALSE);
	DeviceExtension->DeviceThis = this;

	auto scopedIoDeleteDevice = std::experimental::make_scope_exit(
		[&]() { IoDeleteDevice(device_object); });

	if (Asyn_able)
	{
		HANDLE hThread = 0;
		status = PsCreateSystemThread(&hThread,
			THREAD_ALL_ACCESS,
			NULL,
			NULL,
			NULL,
			ddk::nt_device::asyn_thread_routine,
			this);
		if (!NT_SUCCESS(status))
		{
			return false;
		}
		status = ObReferenceObjectByHandle(hThread,
			THREAD_ALL_ACCESS,
			*PsThreadType,
			KernelMode,
			&DeviceExtension->ThreadObject,
			NULL);
		ZwClose(hThread);
		if (!NT_SUCCESS(status))//线程对象获取失败，也许是线程异常，驱动的业务无法完成
		{
			DeviceExtension->bTerminateThread = TRUE;
			KeSetEvent(&DeviceExtension->RequestEvent, 0, FALSE);
			return false;
		}
	}
	status = IoCreateSymbolicLink(&nsDosName,&nsDeviceName);
	if (!NT_SUCCESS(status)) {
		return false;
	}
	auto scopedIoDeleteSymbolicLink = std::experimental::make_scope_exit(
		[&]() { IoDeleteSymbolicLink(&nsDosName); });

	for (auto i = 0; i < IRP_MJ_MAXIMUM_FUNCTION+1;i++)
	{
		driver_object->MajorFunction[i] = ddk::nt_device::DeviceIrpProc;
	}
	scopedIoDeleteDevice.release();
	scopedIoDeleteSymbolicLink.release();
	return true;
}

NTSTATUS ddk::nt_device::DeviceIrpProc(
	IN PDEVICE_OBJECT DeviceObject,
	IN PIRP Irp)
{
	auto dev_ext = reinterpret_cast<PDEVICE_EXTENSION>(DeviceObject->DeviceExtension);
	if (dev_ext
		&&dev_ext->Tag=='obj1')
	{
		auto dev_class = reinterpret_cast<nt_device *>(dev_ext->DeviceThis);
		if(!dev_class->is_asyn())
			return dev_class->device_irp(Irp);
		else
		{
			SECURITY_QUALITY_OF_SERVICE SeQ = { 0 };

			if (dev_ext->SecurityClientCtx != NULL)
			{
				SeDeleteClientSecurity(dev_ext->SecurityClientCtx);
			}
			else
			{
				dev_ext->SecurityClientCtx = (PSECURITY_CLIENT_CONTEXT)malloc(sizeof(SECURITY_CLIENT_CONTEXT));
			}

			RtlZeroMemory(&SeQ, sizeof(SECURITY_QUALITY_OF_SERVICE));

			SeQ.Length = sizeof(SECURITY_QUALITY_OF_SERVICE);
			SeQ.ImpersonationLevel = SecurityImpersonation;
			SeQ.ContextTrackingMode = SECURITY_STATIC_TRACKING;
			SeQ.EffectiveOnly = FALSE;

			SeCreateClientSecurity(
				PsGetCurrentThread(),
				&SeQ,
				FALSE,
				dev_ext->SecurityClientCtx
				);

			IoMarkIrpPending(Irp);

			ExInterlockedInsertTailList(&dev_ext->ListHead, &Irp->Tail.Overlay.ListEntry, &dev_ext->ListLock);

			KeSetEvent(&dev_ext->RequestEvent, 0, FALSE);

			return STATUS_PENDING;
		}
	}
	return STATUS_UNSUCCESSFUL;
}

NTSTATUS ddk::nt_device::device_irp(PIRP Irp)
{
	auto status = STATUS_SUCCESS;
	auto infomation = ULONG_PTR(0);
	auto ioStackIrp = IoGetCurrentIrpStackLocation(Irp);
	if (ioStackIrp)
	{
		auto maj_code = ioStackIrp->MajorFunction;
		auto maj_callback = map_irp_routine.find(maj_code);
		if (maj_callback!=map_irp_routine.end())
		{
			auto func = map_irp_routine[maj_code];
			return func(device_object, Irp);
		}
		else
		{
			//检测是否是IRP_MJ_DEVICE_CONTROL
			if (maj_code==IRP_MJ_DEVICE_CONTROL)
			{
				auto inputBuffer = Irp->AssociatedIrp.SystemBuffer;
				auto outputBuffer = inputBuffer;
				auto ioControlCode = ioStackIrp->Parameters.DeviceIoControl.IoControlCode;
				auto inputBufferLength =
					ioStackIrp->Parameters.DeviceIoControl.InputBufferLength;
				auto outputBufferLength =
					ioStackIrp->Parameters.DeviceIoControl.OutputBufferLength;

				if (map_ioctrl.find(ioControlCode) != map_ioctrl.end())
				{
					switch (METHOD_FROM_CTL_CODE(ioControlCode))
					{
					case METHOD_NEITHER:
						inputBuffer = ioStackIrp->Parameters.DeviceIoControl.Type3InputBuffer;
						outputBuffer = Irp->UserBuffer;
						break;
					case METHOD_BUFFERED:
						break;
					case METHOD_IN_DIRECT:
					case METHOD_OUT_DIRECT:
						outputBuffer = Irp->MdlAddress ? MmGetSystemAddressForMdlSafe(Irp->MdlAddress, NormalPagePriority|MdlMappingNoExecute) : nullptr;
						break;
					}
					auto func = map_ioctrl[ioControlCode];
					status = func(inputBuffer, inputBufferLength, outputBuffer, outputBufferLength, &infomation);
				}
				else
				{
					status = STATUS_NOT_IMPLEMENTED;
				}
			}
		}
	}

	Irp->IoStatus.Status = status;
	Irp->IoStatus.Information = infomation;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return status;
}

NTSTATUS ddk::nt_device::default_irp_routine(PDEVICE_OBJECT devobj, PIRP Irp)
{
	UNREFERENCED_PARAMETER(devobj);
	Irp->IoStatus.Status = STATUS_SUCCESS;
	Irp->IoStatus.Information = 0;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}

VOID ddk::nt_device::asyn_thread_routine(PVOID context)
{
	auto classV = reinterpret_cast<nt_device *>(context);
	classV->asyn_thread_work();
}
void ddk::nt_device::asyn_thread_work()
{
	PDEVICE_EXTENSION   DeviceExtension;
	PLIST_ENTRY         Request;
	PIRP                Irp;

	DeviceExtension = (PDEVICE_EXTENSION)device_object->DeviceExtension;

	KeSetPriorityThread(KeGetCurrentThread(), LOW_REALTIME_PRIORITY);//设置线程运行于低优先级,否则会一卡一卡的！！

	KeLowerIrql(PASSIVE_LEVEL);

	AdjustPrivilege(SE_IMPERSONATE_PRIVILEGE, TRUE);

	while (TRUE)
	{
		KeWaitForSingleObject(&DeviceExtension->RequestEvent,
			Executive,
			KernelMode,
			FALSE,
			NULL);

		if (DeviceExtension->bTerminateThread)
		{
			PsTerminateSystemThread(STATUS_SUCCESS);//终止线程
		}
#pragma warning(push)
#pragma warning(disable:4706)
		Request = ExInterlockedRemoveHeadList(&DeviceExtension->ListHead, &DeviceExtension->ListLock);
		while (Request)
		{
			Irp = CONTAINING_RECORD(Request, IRP, Tail.Overlay.ListEntry);
			SeImpersonateClient(DeviceExtension->SecurityClientCtx, NULL);
			device_irp(Irp);
			PsRevertToSelf();
			Request = ExInterlockedRemoveHeadList(&DeviceExtension->ListHead, &DeviceExtension->ListLock);
		}
#pragma warning(pop)
	}
}
 
NTSTATUS ddk::nt_device::
AdjustPrivilege(
	IN ULONG    Privilege,
	IN BOOLEAN  Enable
	)
{
	NTSTATUS            status;
	HANDLE              token_handle;
	TOKEN_PRIVILEGES    token_privileges;

	status = ZwOpenProcessToken(
		NtCurrentProcess(),
		TOKEN_ALL_ACCESS,
		&token_handle
		);

	if (!NT_SUCCESS(status))
	{
		return status;
	}

	token_privileges.PrivilegeCount = 1;
	token_privileges.Privileges[0].Luid = RtlConvertUlongToLuid(Privilege);
	token_privileges.Privileges[0].Attributes = Enable ? SE_PRIVILEGE_ENABLED : 0;

	status = NtAdjustPrivilegesToken(
		token_handle,
		FALSE,
		&token_privileges,
		sizeof(token_privileges),
		NULL,
		NULL
		);

	ZwClose(token_handle);

	return status;
}

bool ddk::nt_device::is_asyn()
{
	return Asyn_able;
}
