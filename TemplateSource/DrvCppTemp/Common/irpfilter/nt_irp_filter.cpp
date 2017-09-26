#include "stdafx.h"

namespace ddk
{
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
		)
	{
		auto dev_ext = reinterpret_cast<ddk::nt_attach_filter::filter_dev_ext*>(DeviceObject->DeviceExtension);
		if (dev_ext&&dev_ext->Tag == 'flt1')
		{
			auto pThis = reinterpret_cast<ddk::nt_attach_filter*>(dev_ext->ThisCtx);
			auto nextDev = dev_ext->LowerDevice;
			auto fastio = nextDev->DriverObject->FastIoDispatch;
			auto offset = FIELD_OFFSET(FAST_IO_DISPATCH, FastIoCheckIfPossible);
			auto pfunc = reinterpret_cast<PFAST_IO_CHECK_IF_POSSIBLE>(pThis->getFastIoFilter(offset));
			if (fastio)
			{
				if (VALID_FAST_IO_DISPATCH_HANDLER(fastio, FastIoCheckIfPossible))
				{
					if (pfunc)
					{
						return pfunc(
							FileObject,
							FileOffset,
							Length,
							Wait,
							LockKey,
							CheckForReadOperation,
							IoStatus,
							nextDev);
					}

					return (fastio->FastIoCheckIfPossible)(
						FileObject,
						FileOffset,
						Length,
						Wait,
						LockKey,
						CheckForReadOperation,
						IoStatus,
						nextDev);
				}
			}
		}
		return FALSE;
	}
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
		)
	{
		auto dev_ext = reinterpret_cast<ddk::nt_attach_filter::filter_dev_ext*>(DeviceObject->DeviceExtension);
		if (dev_ext&&dev_ext->Tag == 'flt1')
		{
			auto pThis = reinterpret_cast<ddk::nt_attach_filter*>(dev_ext->ThisCtx);
			auto nextDev = dev_ext->LowerDevice;
			auto fastio = nextDev->DriverObject->FastIoDispatch;
			auto offset = FIELD_OFFSET(FAST_IO_DISPATCH, FastIoRead);
			auto pfunc = reinterpret_cast<PFAST_IO_READ>(pThis->getFastIoFilter(offset));
			if (fastio)
			{
				if (VALID_FAST_IO_DISPATCH_HANDLER(fastio, FastIoRead))
				{
					if (pfunc)
					{
						return pfunc(
							FileObject,
							FileOffset,
							Length,
							Wait,
							LockKey,
							Buffer,
							IoStatus,
							nextDev);
					}
					return (fastio->FastIoRead)(
						FileObject,
						FileOffset,
						Length,
						Wait,
						LockKey,
						Buffer,
						IoStatus,
						nextDev);
				}
			}
		}
		return FALSE;
	}
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
		)
	{
		auto dev_ext = reinterpret_cast<ddk::nt_attach_filter::filter_dev_ext*>(DeviceObject->DeviceExtension);
		if (dev_ext&&dev_ext->Tag == 'flt1')
		{
			auto pThis = reinterpret_cast<ddk::nt_attach_filter*>(dev_ext->ThisCtx);
			auto nextDeviceObject = dev_ext->LowerDevice;
			auto fastio = nextDeviceObject->DriverObject->FastIoDispatch;
			auto offset = FIELD_OFFSET(FAST_IO_DISPATCH, FastIoWrite);
			auto pfunc = reinterpret_cast<PFAST_IO_WRITE>(pThis->getFastIoFilter(offset));
			if (fastio)
			{
				if (VALID_FAST_IO_DISPATCH_HANDLER(fastio, FastIoWrite))
				{
					if (pfunc)
					{
						return pfunc(
							FileObject,
							FileOffset,
							Length,
							Wait,
							LockKey,
							Buffer,
							IoStatus,
							nextDeviceObject);
					}
					return (fastio->FastIoWrite)(
						FileObject,
						FileOffset,
						Length,
						Wait,
						LockKey,
						Buffer,
						IoStatus,
						nextDeviceObject);
				}
			}
		}
		return FALSE;
	}
	BOOLEAN
		SfFastIoQueryBasicInfo(
			IN PFILE_OBJECT FileObject,
			IN BOOLEAN Wait,
			OUT PFILE_BASIC_INFORMATION Buffer,
			OUT PIO_STATUS_BLOCK IoStatus,
			IN PDEVICE_OBJECT DeviceObject
		)
	{
		auto dev_ext = reinterpret_cast<ddk::nt_attach_filter::filter_dev_ext*>(DeviceObject->DeviceExtension);
		if (dev_ext&&dev_ext->Tag == 'flt1')
		{
			auto pThis = reinterpret_cast<ddk::nt_attach_filter*>(dev_ext->ThisCtx);
			auto nextDeviceObject = dev_ext->LowerDevice;
			auto fastio = nextDeviceObject->DriverObject->FastIoDispatch;
			auto offset = FIELD_OFFSET(FAST_IO_DISPATCH, FastIoQueryBasicInfo);
			auto pfunc = reinterpret_cast<PFAST_IO_QUERY_BASIC_INFO>(pThis->getFastIoFilter(offset));
			if (fastio)
			{
				if (VALID_FAST_IO_DISPATCH_HANDLER(fastio, FastIoQueryBasicInfo))
				{
					if (pfunc)
					{
						return pfunc(
							FileObject,
							Wait,
							Buffer,
							IoStatus,
							nextDeviceObject);
					}
					return (fastio->FastIoQueryBasicInfo)(
						FileObject,
						Wait,
						Buffer,
						IoStatus,
						nextDeviceObject);
				}
			}
		}
		return FALSE;
	}
	BOOLEAN
		SfFastIoQueryStandardInfo(
			IN PFILE_OBJECT FileObject,
			IN BOOLEAN Wait,
			OUT PFILE_STANDARD_INFORMATION Buffer,
			OUT PIO_STATUS_BLOCK IoStatus,
			IN PDEVICE_OBJECT DeviceObject
		)
	{
		auto dev_ext = reinterpret_cast<ddk::nt_attach_filter::filter_dev_ext*>(DeviceObject->DeviceExtension);
		if (dev_ext&&dev_ext->Tag == 'flt1')
		{
			auto pThis = reinterpret_cast<ddk::nt_attach_filter*>(dev_ext->ThisCtx);
			auto nextDeviceObject = dev_ext->LowerDevice;
			auto fastio = nextDeviceObject->DriverObject->FastIoDispatch;
			auto offset = FIELD_OFFSET(FAST_IO_DISPATCH, FastIoQueryStandardInfo);
			auto pfunc = reinterpret_cast<PFAST_IO_QUERY_STANDARD_INFO>(pThis->getFastIoFilter(offset));
			if (fastio)
			{
				if (VALID_FAST_IO_DISPATCH_HANDLER(fastio, FastIoQueryStandardInfo))
				{
					if (pfunc)
					{
						return pfunc(
							FileObject,
							Wait,
							Buffer,
							IoStatus,
							nextDeviceObject);
					}
					return (fastio->FastIoQueryStandardInfo)(
						FileObject,
						Wait,
						Buffer,
						IoStatus,
						nextDeviceObject);
				}
			}
		}
		return FALSE;
	}
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
		)
	{
		auto dev_ext = reinterpret_cast<ddk::nt_attach_filter::filter_dev_ext*>(DeviceObject->DeviceExtension);
		if (dev_ext&&dev_ext->Tag == 'flt1')
		{
			auto pThis = reinterpret_cast<ddk::nt_attach_filter*>(dev_ext->ThisCtx);
			auto nextDeviceObject = dev_ext->LowerDevice;
			auto fastio = nextDeviceObject->DriverObject->FastIoDispatch;
			auto offset = FIELD_OFFSET(FAST_IO_DISPATCH, FastIoLock);
			auto pfunc = reinterpret_cast<PFAST_IO_LOCK>(pThis->getFastIoFilter(offset));
			if (fastio)
			{
				if (VALID_FAST_IO_DISPATCH_HANDLER(fastio, FastIoLock))
				{
					if (pfunc)
					{
						return pfunc(
							FileObject,
							FileOffset,
							Length,
							ProcessId,
							Key,
							FailImmediately,
							ExclusiveLock,
							IoStatus,
							nextDeviceObject);
					}
					return (fastio->FastIoLock)(
						FileObject,
						FileOffset,
						Length,
						ProcessId,
						Key,
						FailImmediately,
						ExclusiveLock,
						IoStatus,
						nextDeviceObject);
				}
			}
		}
		return FALSE;
	}
	BOOLEAN
		SfFastIoUnlockSingle(
			IN PFILE_OBJECT FileObject,
			IN PLARGE_INTEGER FileOffset,
			IN PLARGE_INTEGER Length,
			PEPROCESS ProcessId,
			ULONG Key,
			OUT PIO_STATUS_BLOCK IoStatus,
			IN PDEVICE_OBJECT DeviceObject
		)
	{
		auto dev_ext = reinterpret_cast<ddk::nt_attach_filter::filter_dev_ext*>(DeviceObject->DeviceExtension);
		if (dev_ext&&dev_ext->Tag == 'flt1')
		{
			auto pThis = reinterpret_cast<ddk::nt_attach_filter*>(dev_ext->ThisCtx);
			auto nextDeviceObject = dev_ext->LowerDevice;
			auto fastio = nextDeviceObject->DriverObject->FastIoDispatch;
			auto offset = FIELD_OFFSET(FAST_IO_DISPATCH, FastIoUnlockSingle);
			auto pfunc = reinterpret_cast<PFAST_IO_UNLOCK_SINGLE>(pThis->getFastIoFilter(offset));
			if (fastio)
			{
				if (VALID_FAST_IO_DISPATCH_HANDLER(fastio, FastIoUnlockSingle))
				{
					if (pfunc)
					{
						return pfunc(
							FileObject,
							FileOffset,
							Length,
							ProcessId,
							Key,
							IoStatus,
							nextDeviceObject);
					}
					return (fastio->FastIoUnlockSingle)(
						FileObject,
						FileOffset,
						Length,
						ProcessId,
						Key,
						IoStatus,
						nextDeviceObject);
				}
			}
		}
		return FALSE;
	}
	BOOLEAN
		SfFastIoUnlockAll(
			IN PFILE_OBJECT FileObject,
			PEPROCESS ProcessId,
			OUT PIO_STATUS_BLOCK IoStatus,
			IN PDEVICE_OBJECT DeviceObject
		)
	{
		auto dev_ext = reinterpret_cast<ddk::nt_attach_filter::filter_dev_ext*>(DeviceObject->DeviceExtension);
		if (dev_ext&&dev_ext->Tag == 'flt1')
		{
			auto pThis = reinterpret_cast<ddk::nt_attach_filter*>(dev_ext->ThisCtx);
			auto nextDeviceObject = dev_ext->LowerDevice;
			auto fastio = nextDeviceObject->DriverObject->FastIoDispatch;
			auto offset = FIELD_OFFSET(FAST_IO_DISPATCH, FastIoUnlockAll);
			auto pfunc = reinterpret_cast<PFAST_IO_UNLOCK_ALL>(pThis->getFastIoFilter(offset));
			if (fastio)
			{
				if (VALID_FAST_IO_DISPATCH_HANDLER(fastio, FastIoUnlockAll))
				{
					if (pfunc)
					{
						return pfunc(
							FileObject,
							ProcessId,
							IoStatus,
							nextDeviceObject);
					}
					return (fastio->FastIoUnlockAll)(
						FileObject,
						ProcessId,
						IoStatus,
						nextDeviceObject);
				}
			}
		}
		return FALSE;
	}
	BOOLEAN
		SfFastIoUnlockAllByKey(
			IN PFILE_OBJECT FileObject,
			PVOID ProcessId,
			ULONG Key,
			OUT PIO_STATUS_BLOCK IoStatus,
			IN PDEVICE_OBJECT DeviceObject
		)
	{
		auto dev_ext = reinterpret_cast<ddk::nt_attach_filter::filter_dev_ext*>(DeviceObject->DeviceExtension);
		if (dev_ext&&dev_ext->Tag == 'flt1')
		{
			auto pThis = reinterpret_cast<ddk::nt_attach_filter*>(dev_ext->ThisCtx);
			auto nextDeviceObject = dev_ext->LowerDevice;
			auto fastio = nextDeviceObject->DriverObject->FastIoDispatch;
			auto offset = FIELD_OFFSET(FAST_IO_DISPATCH, FastIoUnlockAllByKey);
			auto pfunc = reinterpret_cast<PFAST_IO_UNLOCK_ALL_BY_KEY>(pThis->getFastIoFilter(offset));
			if (fastio)
			{
				if (VALID_FAST_IO_DISPATCH_HANDLER(fastio, FastIoUnlockAllByKey))
				{
					if (pfunc)
					{
						return pfunc(
							FileObject,
							ProcessId,
							Key,
							IoStatus,
							nextDeviceObject);
					}
					return (fastio->FastIoUnlockAllByKey)(
						FileObject,
						ProcessId,
						Key,
						IoStatus,
						nextDeviceObject);
				}
			}
		}
		return FALSE;
	}
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
		)
	{
		auto dev_ext = reinterpret_cast<ddk::nt_attach_filter::filter_dev_ext*>(DeviceObject->DeviceExtension);
		if (dev_ext&&dev_ext->Tag == 'flt1')
		{
			auto pThis = reinterpret_cast<ddk::nt_attach_filter*>(dev_ext->ThisCtx);
			auto nextDeviceObject = dev_ext->LowerDevice;
			auto fastio = nextDeviceObject->DriverObject->FastIoDispatch;
			auto offset = FIELD_OFFSET(FAST_IO_DISPATCH, FastIoDeviceControl);
			auto pfunc = reinterpret_cast<PFAST_IO_DEVICE_CONTROL>(pThis->getFastIoFilter(offset));
			if (fastio)
			{
				if (VALID_FAST_IO_DISPATCH_HANDLER(fastio, FastIoDeviceControl))
				{
					if (pfunc)
					{
						return pfunc(
							FileObject,
							Wait,
							InputBuffer,
							InputBufferLength,
							OutputBuffer,
							OutputBufferLength,
							IoControlCode,
							IoStatus,
							nextDeviceObject);
					}
					return (fastio->FastIoDeviceControl)(
						FileObject,
						Wait,
						InputBuffer,
						InputBufferLength,
						OutputBuffer,
						OutputBufferLength,
						IoControlCode,
						IoStatus,
						nextDeviceObject);
				}
			}
		}
		return FALSE;
	}
	VOID
		SfFastIoDetachDevice(
			IN PDEVICE_OBJECT SourceDevice,
			IN PDEVICE_OBJECT TargetDevice
		)
	{
		IoDetachDevice(TargetDevice);
		IoDeleteDevice(SourceDevice);
	}
	BOOLEAN
		SfFastIoQueryNetworkOpenInfo(
			IN PFILE_OBJECT FileObject,
			IN BOOLEAN Wait,
			OUT PFILE_NETWORK_OPEN_INFORMATION Buffer,
			OUT PIO_STATUS_BLOCK IoStatus,
			IN PDEVICE_OBJECT DeviceObject
		)
	{
		auto dev_ext = reinterpret_cast<ddk::nt_attach_filter::filter_dev_ext*>(DeviceObject->DeviceExtension);
		if (dev_ext&&dev_ext->Tag == 'flt1')
		{
			auto pThis = reinterpret_cast<ddk::nt_attach_filter*>(dev_ext->ThisCtx);
			auto nextDeviceObject = dev_ext->LowerDevice;
			auto fastio = nextDeviceObject->DriverObject->FastIoDispatch;
			auto offset = FIELD_OFFSET(FAST_IO_DISPATCH, FastIoQueryNetworkOpenInfo);
			auto pfunc = reinterpret_cast<PFAST_IO_QUERY_NETWORK_OPEN_INFO>(pThis->getFastIoFilter(offset));
			if (fastio)
			{
				if (VALID_FAST_IO_DISPATCH_HANDLER(fastio, FastIoQueryNetworkOpenInfo))
				{
					if (pfunc)
					{
						return pfunc(
							FileObject,
							Wait,
							Buffer,
							IoStatus,
							nextDeviceObject);
					}
					return (fastio->FastIoQueryNetworkOpenInfo)(
						FileObject,
						Wait,
						Buffer,
						IoStatus,
						nextDeviceObject);
				}
			}
		}
		return FALSE;
	}
	BOOLEAN
		SfFastIoMdlRead(
			IN PFILE_OBJECT FileObject,
			IN PLARGE_INTEGER FileOffset,
			IN ULONG Length,
			IN ULONG LockKey,
			OUT PMDL *MdlChain,
			OUT PIO_STATUS_BLOCK IoStatus,
			IN PDEVICE_OBJECT DeviceObject
		)
	{
		auto dev_ext = reinterpret_cast<ddk::nt_attach_filter::filter_dev_ext*>(DeviceObject->DeviceExtension);
		if (dev_ext&&dev_ext->Tag == 'flt1')
		{
			auto pThis = reinterpret_cast<ddk::nt_attach_filter*>(dev_ext->ThisCtx);
			auto nextDeviceObject = dev_ext->LowerDevice;
			auto fastio = nextDeviceObject->DriverObject->FastIoDispatch;
			auto offset = FIELD_OFFSET(FAST_IO_DISPATCH, MdlRead);
			auto pfunc = reinterpret_cast<PFAST_IO_MDL_READ>(pThis->getFastIoFilter(offset));
			if (fastio)
			{
				if (VALID_FAST_IO_DISPATCH_HANDLER(fastio, MdlRead))
				{
					if (pfunc)
					{
						return pfunc(
							FileObject,
							FileOffset,
							Length,
							LockKey,
							MdlChain,
							IoStatus,
							nextDeviceObject);
					}
					return (fastio->MdlRead)(
						FileObject,
						FileOffset,
						Length,
						LockKey,
						MdlChain,
						IoStatus,
						nextDeviceObject);
				}
			}
		}
		return FALSE;
	}
	BOOLEAN
		SfFastIoMdlReadComplete(
			IN PFILE_OBJECT FileObject,
			IN PMDL MdlChain,
			IN PDEVICE_OBJECT DeviceObject
		)
	{
		auto dev_ext = reinterpret_cast<ddk::nt_attach_filter::filter_dev_ext*>(DeviceObject->DeviceExtension);
		if (dev_ext&&dev_ext->Tag == 'flt1')
		{
			auto pThis = reinterpret_cast<ddk::nt_attach_filter*>(dev_ext->ThisCtx);
			auto nextDeviceObject = dev_ext->LowerDevice;
			auto fastio = nextDeviceObject->DriverObject->FastIoDispatch;
			auto offset = FIELD_OFFSET(FAST_IO_DISPATCH, MdlReadComplete);
			auto pfunc = reinterpret_cast<PFAST_IO_MDL_READ_COMPLETE>(pThis->getFastIoFilter(offset));
			if (fastio)
			{
				if (VALID_FAST_IO_DISPATCH_HANDLER(fastio, MdlReadComplete))
				{
					if (pfunc)
					{
						return pfunc(
							FileObject,
							MdlChain,
							nextDeviceObject);
					}
					return (fastio->MdlReadComplete)(
						FileObject,
						MdlChain,
						nextDeviceObject);
				}
			}
		}
		return FALSE;
	}
	BOOLEAN
		SfFastIoPrepareMdlWrite(
			IN PFILE_OBJECT FileObject,
			IN PLARGE_INTEGER FileOffset,
			IN ULONG Length,
			IN ULONG LockKey,
			OUT PMDL *MdlChain,
			OUT PIO_STATUS_BLOCK IoStatus,
			IN PDEVICE_OBJECT DeviceObject
		)
	{
		auto dev_ext = reinterpret_cast<ddk::nt_attach_filter::filter_dev_ext*>(DeviceObject->DeviceExtension);
		if (dev_ext&&dev_ext->Tag == 'flt1')
		{
			auto pThis = reinterpret_cast<ddk::nt_attach_filter*>(dev_ext->ThisCtx);
			auto nextDeviceObject = dev_ext->LowerDevice;
			auto fastio = nextDeviceObject->DriverObject->FastIoDispatch;
			auto offset = FIELD_OFFSET(FAST_IO_DISPATCH, PrepareMdlWrite);
			auto pfunc = reinterpret_cast<PFAST_IO_PREPARE_MDL_WRITE>(pThis->getFastIoFilter(offset));
			if (fastio)
			{
				if (VALID_FAST_IO_DISPATCH_HANDLER(fastio, PrepareMdlWrite))
				{
					if (pfunc)
					{
						return pfunc(
							FileObject,
							FileOffset,
							Length,
							LockKey,
							MdlChain,
							IoStatus,
							nextDeviceObject);
					}
					return (fastio->PrepareMdlWrite)(
						FileObject,
						FileOffset,
						Length,
						LockKey,
						MdlChain,
						IoStatus,
						nextDeviceObject);
				}
			}
		}
		return FALSE;
	}
	BOOLEAN
		SfFastIoMdlWriteComplete(
			IN PFILE_OBJECT FileObject,
			IN PLARGE_INTEGER FileOffset,
			IN PMDL MdlChain,
			IN PDEVICE_OBJECT DeviceObject
		)
	{
		auto dev_ext = reinterpret_cast<ddk::nt_attach_filter::filter_dev_ext*>(DeviceObject->DeviceExtension);
		if (dev_ext&&dev_ext->Tag == 'flt1')
		{
			auto pThis = reinterpret_cast<ddk::nt_attach_filter*>(dev_ext->ThisCtx);
			auto nextDeviceObject = dev_ext->LowerDevice;
			auto fastio = nextDeviceObject->DriverObject->FastIoDispatch;
			auto offset = FIELD_OFFSET(FAST_IO_DISPATCH, MdlWriteComplete);
			auto pfunc = reinterpret_cast<PFAST_IO_MDL_WRITE_COMPLETE>(pThis->getFastIoFilter(offset));
			if (fastio)
			{
				if (VALID_FAST_IO_DISPATCH_HANDLER(fastio, MdlWriteComplete))
				{
					if (pfunc)
					{
						return pfunc(
							FileObject,
							FileOffset,
							MdlChain,
							nextDeviceObject);
					}
					return (fastio->MdlWriteComplete)(
						FileObject,
						FileOffset,
						MdlChain,
						nextDeviceObject);
				}
			}
		}
		return FALSE;
	}
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
		)
	{
		auto dev_ext = reinterpret_cast<ddk::nt_attach_filter::filter_dev_ext*>(DeviceObject->DeviceExtension);
		if (dev_ext&&dev_ext->Tag == 'flt1')
		{
			auto pThis = reinterpret_cast<ddk::nt_attach_filter*>(dev_ext->ThisCtx);
			auto nextDeviceObject = dev_ext->LowerDevice;
			auto fastio = nextDeviceObject->DriverObject->FastIoDispatch;
			auto offset = FIELD_OFFSET(FAST_IO_DISPATCH, FastIoReadCompressed);
			auto pfunc = reinterpret_cast<PFAST_IO_READ_COMPRESSED>(pThis->getFastIoFilter(offset));
			if (fastio)
			{
				if (VALID_FAST_IO_DISPATCH_HANDLER(fastio, FastIoReadCompressed))
				{
					if (pfunc)
					{
						return pfunc(
							FileObject,
							FileOffset,
							Length,
							LockKey,
							Buffer,
							MdlChain,
							IoStatus,
							CompressedDataInfo,
							CompressedDataInfoLength,
							nextDeviceObject);
					}
					return (fastio->FastIoReadCompressed)(
						FileObject,
						FileOffset,
						Length,
						LockKey,
						Buffer,
						MdlChain,
						IoStatus,
						CompressedDataInfo,
						CompressedDataInfoLength,
						nextDeviceObject);
				}
			}
		}
		return FALSE;
	}
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
		)
	{
		auto dev_ext = reinterpret_cast<ddk::nt_attach_filter::filter_dev_ext*>(DeviceObject->DeviceExtension);
		if (dev_ext&&dev_ext->Tag == 'flt1')
		{
			auto pThis = reinterpret_cast<ddk::nt_attach_filter*>(dev_ext->ThisCtx);
			auto nextDeviceObject = dev_ext->LowerDevice;
			auto fastio = nextDeviceObject->DriverObject->FastIoDispatch;
			auto offset = FIELD_OFFSET(FAST_IO_DISPATCH, FastIoWriteCompressed);
			auto pfunc = reinterpret_cast<PFAST_IO_WRITE_COMPRESSED>(pThis->getFastIoFilter(offset));
			if (fastio)
			{
				if (VALID_FAST_IO_DISPATCH_HANDLER(fastio, FastIoWriteCompressed))
				{
					if (pfunc)
					{
						return pfunc(
							FileObject,
							FileOffset,
							Length,
							LockKey,
							Buffer,
							MdlChain,
							IoStatus,
							CompressedDataInfo,
							CompressedDataInfoLength,
							nextDeviceObject);
					}
					return (fastio->FastIoWriteCompressed)(
						FileObject,
						FileOffset,
						Length,
						LockKey,
						Buffer,
						MdlChain,
						IoStatus,
						CompressedDataInfo,
						CompressedDataInfoLength,
						nextDeviceObject);
				}
			}
		}
		return FALSE;
	}
	BOOLEAN
		SfFastIoMdlReadCompleteCompressed(
			IN PFILE_OBJECT FileObject,
			IN PMDL MdlChain,
			IN PDEVICE_OBJECT DeviceObject
		)
	{
		auto dev_ext = reinterpret_cast<ddk::nt_attach_filter::filter_dev_ext*>(DeviceObject->DeviceExtension);
		if (dev_ext&&dev_ext->Tag == 'flt1')
		{
			auto pThis = reinterpret_cast<ddk::nt_attach_filter*>(dev_ext->ThisCtx);
			auto nextDeviceObject = dev_ext->LowerDevice;
			auto fastio = nextDeviceObject->DriverObject->FastIoDispatch;
			auto offset = FIELD_OFFSET(FAST_IO_DISPATCH, MdlReadCompleteCompressed);
			auto pfunc = reinterpret_cast<PFAST_IO_MDL_READ_COMPLETE_COMPRESSED>(pThis->getFastIoFilter(offset));
			if (fastio)
			{
				if (VALID_FAST_IO_DISPATCH_HANDLER(fastio, MdlReadCompleteCompressed))
				{
					if (pfunc)
					{
						return pfunc(
							FileObject,
							MdlChain,
							nextDeviceObject);
					}
					return (fastio->MdlReadCompleteCompressed)(
						FileObject,
						MdlChain,
						nextDeviceObject);
				}
			}
		}
		return FALSE;
	}
	BOOLEAN
		SfFastIoMdlWriteCompleteCompressed(
			IN PFILE_OBJECT FileObject,
			IN PLARGE_INTEGER FileOffset,
			IN PMDL MdlChain,
			IN PDEVICE_OBJECT DeviceObject
		)
	{
		auto dev_ext = reinterpret_cast<ddk::nt_attach_filter::filter_dev_ext*>(DeviceObject->DeviceExtension);
		if (dev_ext&&dev_ext->Tag == 'flt1')
		{
			auto pThis = reinterpret_cast<ddk::nt_attach_filter*>(dev_ext->ThisCtx);
			auto nextDeviceObject = dev_ext->LowerDevice;
			auto fastio = nextDeviceObject->DriverObject->FastIoDispatch;
			auto offset = FIELD_OFFSET(FAST_IO_DISPATCH, MdlWriteCompleteCompressed);
			auto pfunc = reinterpret_cast<PFAST_IO_MDL_WRITE_COMPLETE_COMPRESSED>(pThis->getFastIoFilter(offset));
			if (fastio)
			{
				if (VALID_FAST_IO_DISPATCH_HANDLER(fastio, MdlWriteCompleteCompressed))
				{
					if (pfunc)
					{
						return pfunc(
							FileObject,
							FileOffset,
							MdlChain,
							nextDeviceObject);
					}
					return (fastio->MdlWriteCompleteCompressed)(
						FileObject,
						FileOffset,
						MdlChain,
						nextDeviceObject);
				}
			}
		}
		return FALSE;
	}
	BOOLEAN
		SfFastIoQueryOpen(
			IN PIRP Irp,
			OUT PFILE_NETWORK_OPEN_INFORMATION NetworkInformation,
			IN PDEVICE_OBJECT DeviceObject
		)
	{
		auto dev_ext = reinterpret_cast<ddk::nt_attach_filter::filter_dev_ext*>(DeviceObject->DeviceExtension);
		if (dev_ext&&dev_ext->Tag == 'flt1')
		{
			auto pThis = reinterpret_cast<ddk::nt_attach_filter*>(dev_ext->ThisCtx);
			auto nextDeviceObject = dev_ext->LowerDevice;
			auto fastio = nextDeviceObject->DriverObject->FastIoDispatch;
			auto offset = FIELD_OFFSET(FAST_IO_DISPATCH, FastIoQueryOpen);
			auto pfunc = reinterpret_cast<PFAST_IO_QUERY_OPEN>(pThis->getFastIoFilter(offset));
			if (fastio)
			{
				if (VALID_FAST_IO_DISPATCH_HANDLER(fastio, FastIoQueryOpen))
				{
					PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation(Irp);
					irpSp->DeviceObject = nextDeviceObject;

					BOOLEAN res;
					if (pfunc)
					{
						res = pfunc(
							Irp,
							NetworkInformation,
							nextDeviceObject);
					}
					else
					{
						res = (fastio->FastIoQueryOpen)(
							Irp,
							NetworkInformation,
							nextDeviceObject);
					}
					irpSp->DeviceObject = DeviceObject;

					return res;
				}
			}
		}
		return FALSE;
	}
};