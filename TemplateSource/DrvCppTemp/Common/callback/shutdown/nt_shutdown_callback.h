#pragma once
namespace ddk::callback
{
	class nt_shutdown_callback
	{
	public:
		using _shutdown_ctx = struct 
		{
			DWORD32 Tag;
			PVOID pThis;
		};
		using pfn_shutdown_callback = std::function<void(PVOID)>;
		nt_shutdown_callback() {
			pDrvObject = nullptr;
			pDevObject = nullptr;
			pExtDevice = nullptr;
			_pfnCallback = nullptr;
		}
		~nt_shutdown_callback() {
			
			if (pDevObject)
			{
				IoUnregisterShutdownNotification(pDevObject);
				IoDeleteDevice(pDevObject);
			}
			if (pDrvObject)
			{
				ddk::nt_drivers::getInstance().del_driver_obj(pDrvObject);
			}
		}
	public:
		bool init()
		{
			//DriverObject
			//DeviceObject
			//特殊的Irp
			pDrvObject = nullptr;
			pDevObject = nullptr;
			pExtDevice = nullptr;
			_pfnCallback = nullptr;
			pDrvObject = ddk::nt_drivers::getInstance().get_new_driver();
			if (!pDrvObject)
			{
				return false;
			}
			auto ns = IoCreateDevice(pDrvObject,
				sizeof(_shutdown_ctx),
				nullptr, 
				FILE_DEVICE_UNKNOWN,
				FILE_DEVICE_SECURE_OPEN,
				FALSE,
				&pDevObject);
			if (!NT_SUCCESS(ns))
			{
				return false;
			}
			pExtDevice = (_shutdown_ctx *)pDevObject->DeviceExtension;

			pDevObject->Flags &= ~DO_DEVICE_INITIALIZING;//不在DriverEntry流程里，也就是不收IO管理器控制的DeviceObject创建需要自己清除init标记

			pDevObject->Flags |= DO_DIRECT_IO;//I/O时使用MmGetSystemAddressForMdlSafe得到buffer
			RtlZeroMemory(pExtDevice, sizeof(_shutdown_ctx));
			pExtDevice->pThis = this;
			pExtDevice->Tag = 'shut';
			pDrvObject->MajorFunction[IRP_MJ_SHUTDOWN] = ddk::callback::nt_shutdown_callback::ShutDownRoutine;
			ns = IoRegisterShutdownNotification(pDevObject);
			if (!NT_SUCCESS(ns))
			{
				return false;
			}
			return true;
		}
	private:
		PDEVICE_OBJECT pDevObject;
		PDRIVER_OBJECT pDrvObject;
		_shutdown_ctx *pExtDevice;
		pfn_shutdown_callback _pfnCallback;
		PVOID _Ctx;
	public:
		void OnCallBack()
		{
			if (_pfnCallback)
			{
				_pfnCallback(_Ctx);
			}
		}
	public:
		bool set_function(pfn_shutdown_callback _func, PVOID Context)
		{
			_pfnCallback = _func;
			_Ctx = Context;
			return true;
		}
	public:
		static NTSTATUS
			ShutDownRoutine(
				_In_ struct _DEVICE_OBJECT *DeviceObject,
				_Inout_ struct _IRP *Irp
			)
		{
			NTSTATUS status = STATUS_SUCCESS;
			auto pExt = reinterpret_cast<_shutdown_ctx*>(DeviceObject->DeviceExtension);
			if (pExt)
			{
				if (pExt->Tag=='shut')
				{
					auto pthis = reinterpret_cast<nt_shutdown_callback*>(pExt->pThis);
					if (pthis)
					{
						pthis->OnCallBack();
					}
				}
			}
			Irp->IoStatus.Status = status;
			Irp->IoStatus.Information = 0;
			IoCompleteRequest(Irp, IO_NO_INCREMENT);
			return status;
		}
	};
};