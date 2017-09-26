#pragma once
namespace ddk
{
	//////////////////////////////////////////////////////////////////////////
#define VALID_FAST_IO_DISPATCH_HANDLER(_FastIoDispatchPtr, _FieldName) \
	(((_FastIoDispatchPtr) != NULL) && \
	(((_FastIoDispatchPtr)->SizeOfFastIoDispatch) >= \
	(FIELD_OFFSET(FAST_IO_DISPATCH, _FieldName) + sizeof(void *))) && \
	((_FastIoDispatchPtr)->_FieldName != NULL))

	BOOLEAN
		SfFastIoCheckIfPossible(
			IN PFILE_OBJECT FileObject,
			IN PLARGE_INTEGER FileOffset,
			IN ULONG Length,
			IN BOOLEAN Wait,
			IN ULONG LockKey,
			IN BOOLEAN CheckForReadOperation,
			OUT PIO_STATUS_BLOCK IoStatus,
			IN PDEVICE_OBJECT DeviceObject
		);
	BOOLEAN
		SfFastIoRead(
			IN PFILE_OBJECT FileObject,
			IN PLARGE_INTEGER FileOffset,
			IN ULONG Length,
			IN BOOLEAN Wait,
			IN ULONG LockKey,
			OUT PVOID Buffer,
			OUT PIO_STATUS_BLOCK IoStatus,
			IN PDEVICE_OBJECT DeviceObject
		);
	BOOLEAN
		SfFastIoWrite(
			IN PFILE_OBJECT FileObject,
			IN PLARGE_INTEGER FileOffset,
			IN ULONG Length,
			IN BOOLEAN Wait,
			IN ULONG LockKey,
			IN PVOID Buffer,
			OUT PIO_STATUS_BLOCK IoStatus,
			IN PDEVICE_OBJECT DeviceObject
		);
	BOOLEAN
		SfFastIoQueryBasicInfo(
			IN PFILE_OBJECT FileObject,
			IN BOOLEAN Wait,
			OUT PFILE_BASIC_INFORMATION Buffer,
			OUT PIO_STATUS_BLOCK IoStatus,
			IN PDEVICE_OBJECT DeviceObject
		);
	BOOLEAN
		SfFastIoQueryStandardInfo(
			IN PFILE_OBJECT FileObject,
			IN BOOLEAN Wait,
			OUT PFILE_STANDARD_INFORMATION Buffer,
			OUT PIO_STATUS_BLOCK IoStatus,
			IN PDEVICE_OBJECT DeviceObject
		);
	BOOLEAN
		SfFastIoLock(
			IN PFILE_OBJECT FileObject,
			IN PLARGE_INTEGER FileOffset,
			IN PLARGE_INTEGER Length,
			PEPROCESS ProcessId,
			ULONG Key,
			BOOLEAN FailImmediately,
			BOOLEAN ExclusiveLock,
			OUT PIO_STATUS_BLOCK IoStatus,
			IN PDEVICE_OBJECT DeviceObject
		);
	BOOLEAN
		SfFastIoUnlockSingle(
			IN PFILE_OBJECT FileObject,
			IN PLARGE_INTEGER FileOffset,
			IN PLARGE_INTEGER Length,
			PEPROCESS ProcessId,
			ULONG Key,
			OUT PIO_STATUS_BLOCK IoStatus,
			IN PDEVICE_OBJECT DeviceObject
		);
	BOOLEAN
		SfFastIoUnlockAll(
			IN PFILE_OBJECT FileObject,
			PEPROCESS ProcessId,
			OUT PIO_STATUS_BLOCK IoStatus,
			IN PDEVICE_OBJECT DeviceObject
		);
	BOOLEAN
		SfFastIoUnlockAllByKey(
			IN PFILE_OBJECT FileObject,
			PVOID ProcessId,
			ULONG Key,
			OUT PIO_STATUS_BLOCK IoStatus,
			IN PDEVICE_OBJECT DeviceObject
		);
	BOOLEAN
		SfFastIoDeviceControl(
			IN PFILE_OBJECT FileObject,
			IN BOOLEAN Wait,
			IN PVOID InputBuffer OPTIONAL,
			IN ULONG InputBufferLength,
			OUT PVOID OutputBuffer OPTIONAL,
			IN ULONG OutputBufferLength,
			IN ULONG IoControlCode,
			OUT PIO_STATUS_BLOCK IoStatus,
			IN PDEVICE_OBJECT DeviceObject
		);
	VOID
		SfFastIoDetachDevice(
			IN PDEVICE_OBJECT SourceDevice,
			IN PDEVICE_OBJECT TargetDevice
		);
	BOOLEAN
		SfFastIoQueryNetworkOpenInfo(
			IN PFILE_OBJECT FileObject,
			IN BOOLEAN Wait,
			OUT PFILE_NETWORK_OPEN_INFORMATION Buffer,
			OUT PIO_STATUS_BLOCK IoStatus,
			IN PDEVICE_OBJECT DeviceObject
		);
	BOOLEAN
		SfFastIoMdlRead(
			IN PFILE_OBJECT FileObject,
			IN PLARGE_INTEGER FileOffset,
			IN ULONG Length,
			IN ULONG LockKey,
			OUT PMDL *MdlChain,
			OUT PIO_STATUS_BLOCK IoStatus,
			IN PDEVICE_OBJECT DeviceObject
		);
	BOOLEAN
		SfFastIoMdlReadComplete(
			IN PFILE_OBJECT FileObject,
			IN PMDL MdlChain,
			IN PDEVICE_OBJECT DeviceObject
		);
	BOOLEAN
		SfFastIoPrepareMdlWrite(
			IN PFILE_OBJECT FileObject,
			IN PLARGE_INTEGER FileOffset,
			IN ULONG Length,
			IN ULONG LockKey,
			OUT PMDL *MdlChain,
			OUT PIO_STATUS_BLOCK IoStatus,
			IN PDEVICE_OBJECT DeviceObject
		);
	BOOLEAN
		SfFastIoMdlWriteComplete(
			IN PFILE_OBJECT FileObject,
			IN PLARGE_INTEGER FileOffset,
			IN PMDL MdlChain,
			IN PDEVICE_OBJECT DeviceObject
		);
	BOOLEAN
		SfFastIoReadCompressed(
			IN PFILE_OBJECT FileObject,
			IN PLARGE_INTEGER FileOffset,
			IN ULONG Length,
			IN ULONG LockKey,
			OUT PVOID Buffer,
			OUT PMDL *MdlChain,
			OUT PIO_STATUS_BLOCK IoStatus,
			OUT struct _COMPRESSED_DATA_INFO *CompressedDataInfo,
			IN ULONG CompressedDataInfoLength,
			IN PDEVICE_OBJECT DeviceObject
		);
	BOOLEAN
		SfFastIoWriteCompressed(
			IN PFILE_OBJECT FileObject,
			IN PLARGE_INTEGER FileOffset,
			IN ULONG Length,
			IN ULONG LockKey,
			IN PVOID Buffer,
			OUT PMDL *MdlChain,
			OUT PIO_STATUS_BLOCK IoStatus,
			IN struct _COMPRESSED_DATA_INFO *CompressedDataInfo,
			IN ULONG CompressedDataInfoLength,
			IN PDEVICE_OBJECT DeviceObject
		);
	BOOLEAN
		SfFastIoMdlReadCompleteCompressed(
			IN PFILE_OBJECT FileObject,
			IN PMDL MdlChain,
			IN PDEVICE_OBJECT DeviceObject
		);
	BOOLEAN
		SfFastIoMdlWriteCompleteCompressed(
			IN PFILE_OBJECT FileObject,
			IN PLARGE_INTEGER FileOffset,
			IN PMDL MdlChain,
			IN PDEVICE_OBJECT DeviceObject
		);
	BOOLEAN
		SfFastIoQueryOpen(
			IN PIRP Irp,
			OUT PFILE_NETWORK_OPEN_INFORMATION NetworkInformation,
			IN PDEVICE_OBJECT DeviceObject
		);

	//////////////////////////////////////////////////////////////////////////
};
namespace ddk
{
	class nt_irp_dispatch :public Singleton<nt_irp_dispatch>
	{
	public:
		using do_dispatch_type = std::function<NTSTATUS(PDEVICE_OBJECT,PIRP)>;
		nt_irp_dispatch() {

		}
		~nt_irp_dispatch() {

		}
	public:
		NTSTATUS NTAPI do_dispatch(
			IN PDEVICE_OBJECT DeviceObject,
			IN PIRP Irp)
		{
			auto m_drv = DeviceObject->DriverObject;
			if (m_dispatch.find(m_drv) != m_dispatch.end())
			{
				return m_dispatch[m_drv](DeviceObject, Irp);
			}
			Irp->IoStatus.Status = STATUS_NOT_IMPLEMENTED;
			Irp->IoStatus.Information = 0;
			IoCompleteRequest(Irp, IO_NO_INCREMENT);
			return STATUS_NOT_IMPLEMENTED;
		}
	public:
		void register_dispatch(PDRIVER_OBJECT drv_obj, do_dispatch_type dispatch)
		{
			for (auto i = 0; i < IRP_MJ_MAXIMUM_FUNCTION + 1; i++)
			{
				drv_obj->MajorFunction[i] = ddk::nt_irp_dispatch::DispatchDrv;
			}
			m_dispatch[drv_obj] = dispatch;
		}
	public:
		static NTSTATUS DispatchDrv(
			IN PDEVICE_OBJECT DeviceObject,
			IN PIRP Irp)
		{
			return ddk::nt_irp_dispatch::getInstance().do_dispatch(DeviceObject, Irp);
		}
	private:
		std::map<PDRIVER_OBJECT, do_dispatch_type>m_dispatch;
	};

	class nt_attach_filter
	{
	public:
		using  callback_irp = std::function<NTSTATUS(PDEVICE_OBJECT, PDEVICE_OBJECT, PIRP)>;
		using  filter_dev_ext = struct {
			DWORD32 Tag;
			PDEVICE_OBJECT TargetDevice;    //被Attach的目标
			PDEVICE_OBJECT LowerDevice;	//Attach后的Lower
			PVOID ThisCtx;
		};
		nt_attach_filter() {
			fastIoDispatch = nullptr;
			b_attached = false;
			_self_drv = nullptr;
			_fileobject = nullptr;
			_self_drv = ddk::nt_drivers::getInstance().get_new_driver();
			if (_self_drv)
			{
				ddk::nt_irp_dispatch::getInstance().register_dispatch(_self_drv,
					std::bind(&ddk::nt_attach_filter::do_dispatch, this,
						std::placeholders::_1,
						std::placeholders::_2));

				make_fast_io_dispatch();
			}
		};
		~nt_attach_filter() {
			if (b_attached)
			{
				auto devobj = _self_drv->DeviceObject;
				while (devobj)
				{
					auto pNext = devobj->NextDevice;
					auto pExt = reinterpret_cast<filter_dev_ext*>(devobj->DeviceExtension);
					__try
					{
						IoDetachDevice(pExt->LowerDevice);
						IoDeleteDevice(devobj);
					}
					__except (EXCEPTION_EXECUTE_HANDLER) {}
					devobj = pNext;
				}
			}
			_lock.wait_for_release();
			if (_self_drv)
			{
				if (fastIoDispatch)
				{
					_self_drv->FastIoDispatch = nullptr;
					free(fastIoDispatch);
				}
				ddk::nt_drivers::getInstance().del_driver_obj(_self_drv);
			}
		};
	public:
		bool attach_driver(std::wstring drvName)
		{
			if (!_self_drv)
			{
				return false;
			}
			PDRIVER_OBJECT object = nullptr;
			if (!ddk::util::get_driver_object(drvName, object))
			{
				return false;
			}
			auto pTarget = object->DeviceObject;
			while (pTarget)
			{
				PDEVICE_OBJECT fltobj = nullptr;
				PDEVICE_OBJECT lwrobj = nullptr;
				if (!attach_device(pTarget, &fltobj, &lwrobj)
					)
				{
					return false;
				}
				pTarget = pTarget->NextDevice;
			}
			b_attached = true;

			return true;
		}
		bool attach_device(std::wstring devName)
		{
			if (!_self_drv)
			{
				return false;
			}
			UNICODE_STRING nsDevName;
			PDEVICE_OBJECT devObject = nullptr;
			RtlInitUnicodeString(&nsDevName, devName.c_str());
			b_attached = true;
			auto ns = IoGetDeviceObjectPointer(&nsDevName, FILE_READ_ATTRIBUTES,
				&_fileobject,
				&devObject);
			if (ns == STATUS_SUCCESS)
			{
				PDEVICE_OBJECT filt = nullptr;
				PDEVICE_OBJECT lower = nullptr;
				if (attach_device(devObject, &filt, &lower))
				{
					if (_fileobject)
					{
						ObDereferenceObject(_fileobject);
						_fileobject = nullptr;
					}
					b_attached = true;
					return true;
				}
			}
			return false;
		}
		bool attach_device_obj(PDEVICE_OBJECT DeviceObject)
		{
			if (!_self_drv)
			{
				return false;
			}
			PDEVICE_OBJECT fltobj = nullptr;
			PDEVICE_OBJECT lwrobj = nullptr;
			if (!attach_device(DeviceObject, &fltobj, &lwrobj)
				)
			{
				return false;
			}
			b_attached = true;
			return true;
		}
	private:
		bool attach_device(
			PDEVICE_OBJECT targetDev,
			PDEVICE_OBJECT *filterDev,
			PDEVICE_OBJECT *lowerDev)
		{
			auto ns = IoCreateDevice(_self_drv,
				sizeof(filter_dev_ext),
				nullptr,
				targetDev->DeviceType,
				targetDev->Characteristics,
				FALSE,
				filterDev);
			if (!NT_SUCCESS(ns))
			{
				return false;
			}
			auto filtdev = *filterDev;
			filtdev->Flags &= ~DO_DEVICE_INITIALIZING;
			ns = IoAttachDeviceToDeviceStackSafe(filtdev, targetDev, lowerDev);
			if (ns != STATUS_SUCCESS)
			{
				IoDeleteDevice(*filterDev);
				*filterDev = nullptr;
				return false;
			}
			auto lwrdev = *lowerDev;

			auto devExt = reinterpret_cast<filter_dev_ext*>(filtdev->DeviceExtension);
			if (devExt)
			{
				RtlZeroMemory(devExt, sizeof(filter_dev_ext));
				//InitializeListHead(&devExt->ListHead);
				///*KeInitializeSpinLock(&DeviceExtension->ListLock);*/
				//KeInitializeEvent(&devExt->RequestEvent, SynchronizationEvent, FALSE);
				devExt->Tag = 'flt1';
				devExt->LowerDevice = lwrdev;
				devExt->TargetDevice = targetDev;
				devExt->ThisCtx = this;
			}
			filtdev->DeviceType = lwrdev->DeviceType;
			filtdev->Characteristics = lwrdev->Characteristics;
			filtdev->StackSize = lwrdev->StackSize + 1;
			filtdev->Flags |= lwrdev->Flags & (DO_BUFFERED_IO | DO_DIRECT_IO | DO_POWER_PAGABLE);
			return true;
		}
	protected:
		nt_attach_filter(const nt_attach_filter&) = delete;
		nt_attach_filter & operator = (const nt_attach_filter &) = delete;
	private:
		bool b_attached;
		PDRIVER_OBJECT _self_drv;
		PFILE_OBJECT _fileobject;
		std::map<int, callback_irp> m_maj_routine;
		nt_lock _lock;
		PFAST_IO_DISPATCH fastIoDispatch;
		std::map<int, PVOID>fastIoFilter;
	private:
		void make_fast_io_dispatch()
		{
			fastIoDispatch = reinterpret_cast<PFAST_IO_DISPATCH>(malloc(sizeof(FAST_IO_DISPATCH)));
			if (fastIoDispatch)
			{
				// 内存清零。
				RtlZeroMemory(fastIoDispatch, sizeof(FAST_IO_DISPATCH));
				fastIoDispatch->SizeOfFastIoDispatch = sizeof(FAST_IO_DISPATCH);
				//填写函数接口表
				fastIoDispatch->FastIoCheckIfPossible = SfFastIoCheckIfPossible;
				fastIoDispatch->FastIoRead = SfFastIoRead;
				fastIoDispatch->FastIoWrite = SfFastIoWrite;
				fastIoDispatch->FastIoQueryBasicInfo = SfFastIoQueryBasicInfo;
				fastIoDispatch->FastIoQueryStandardInfo = SfFastIoQueryStandardInfo;
				fastIoDispatch->FastIoLock = SfFastIoLock;
				fastIoDispatch->FastIoUnlockSingle = SfFastIoUnlockSingle;
				fastIoDispatch->FastIoUnlockAll = SfFastIoUnlockAll;
				fastIoDispatch->FastIoUnlockAllByKey = SfFastIoUnlockAllByKey;
				fastIoDispatch->FastIoDeviceControl = SfFastIoDeviceControl;
				fastIoDispatch->FastIoDetachDevice = nullptr;// SfFastIoDetachDevice;
				fastIoDispatch->FastIoQueryNetworkOpenInfo = SfFastIoQueryNetworkOpenInfo;
				fastIoDispatch->MdlRead = SfFastIoMdlRead;
				fastIoDispatch->MdlReadComplete = SfFastIoMdlReadComplete;
				fastIoDispatch->PrepareMdlWrite = SfFastIoPrepareMdlWrite;
				fastIoDispatch->MdlWriteComplete = SfFastIoMdlWriteComplete;
				fastIoDispatch->FastIoReadCompressed = SfFastIoReadCompressed;
				fastIoDispatch->FastIoWriteCompressed = SfFastIoWriteCompressed;
				fastIoDispatch->MdlReadCompleteCompressed = SfFastIoMdlReadCompleteCompressed;
				fastIoDispatch->MdlWriteCompleteCompressed = SfFastIoMdlWriteCompleteCompressed;
				fastIoDispatch->FastIoQueryOpen = SfFastIoQueryOpen;

				_self_drv->FastIoDispatch = fastIoDispatch;
			}
		}
	private:
		NTSTATUS do_pnp(PDEVICE_OBJECT object, PIRP Irp)
		{
			auto irpstack = IoGetCurrentIrpStackLocation(Irp);
			auto ext = reinterpret_cast<filter_dev_ext *>(object->DeviceExtension);
			NTSTATUS ns = STATUS_NOT_IMPLEMENTED;
			switch (irpstack->MinorFunction)
			{
			case IRP_MN_REMOVE_DEVICE:
				IoSkipCurrentIrpStackLocation(Irp);
				IoCallDriver(ext->LowerDevice, Irp);
				IoDetachDevice(ext->LowerDevice);
				IoDeleteDevice(object);
				ns = STATUS_SUCCESS;
				break;
			default:
				IoSkipCurrentIrpStackLocation(Irp);
				ns = IoCallDriver(ext->LowerDevice, Irp);
				break;
			}
			return ns;
		}
		NTSTATUS do_power(PDEVICE_OBJECT object, PIRP Irp)
		{
			auto ext = reinterpret_cast<filter_dev_ext*>(object->DeviceExtension);
			PoStartNextPowerIrp(Irp);
			IoSkipCurrentIrpStackLocation(Irp);
			return PoCallDriver(ext->LowerDevice, Irp);
		}
	public:
		PVOID getFastIoFilter(int offset)
		{
			if (fastIoFilter.find(offset) != fastIoFilter.end())
				return fastIoFilter[offset];
			return nullptr;
		}
		bool set_fast_io_filter(int offset, PVOID filter_func)
		{
			fastIoFilter[offset] = filter_func;
			return true;
		}
		void set_callback(int maj, callback_irp callback)
		{
			m_maj_routine[maj] = callback;
		}
		void acquire()
		{
			_lock.only_acquire();
		}
		void release()
		{
			_lock.release();
		}
	public:
		NTSTATUS do_dispatch(PDEVICE_OBJECT object, PIRP Irp)
		{
			_lock.only_acquire();
			auto exit_lock = std::experimental::make_scope_exit([&]() {
				_lock.release();
			});
			NTSTATUS ns = STATUS_NOT_IMPLEMENTED;
			auto DevExt = reinterpret_cast<filter_dev_ext*>(object->DeviceExtension);
			auto IrpStack = IoGetCurrentIrpStackLocation(Irp);
			auto maj_func = IrpStack->MajorFunction;
			if (!DevExt)
			{
				IoSkipCurrentIrpStackLocation(Irp);
				ns = IoCallDriver(DevExt->LowerDevice, Irp);
				return ns;
			}
			if (DevExt->Tag!='flt1')
			{
				IoSkipCurrentIrpStackLocation(Irp);
				ns = IoCallDriver(DevExt->LowerDevice, Irp);
				return ns;
			}
			if (m_maj_routine.find(maj_func) != m_maj_routine.end())
			{
				ns = m_maj_routine[maj_func](DevExt->LowerDevice, object, Irp);
			}
			else
			{
				switch (maj_func)
				{
				case IRP_MJ_PNP_POWER:
					ns = do_pnp(object, Irp);
					break;
				case IRP_MJ_POWER:
					ns = do_power(object, Irp);
					break;
				default:
					IoSkipCurrentIrpStackLocation(Irp);
					ns = IoCallDriver(DevExt->LowerDevice, Irp);
				}
			}
			return ns;
		}

	};
};