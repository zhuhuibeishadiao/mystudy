#pragma once
namespace ddk
{
	class nt_drivers :public Singleton<nt_drivers>
	{
	public:
		nt_drivers() {
			m_count = 0;
		}
		PDRIVER_OBJECT get_new_driver()
		{
			GUID Attach;
			wchar_t szGuid[MAX_PATH] = { 0 };
			_lock.acquire();
			auto lock_free = std::experimental::make_scope_exit([&]() {
				_lock.release();
			});
			auto ns = ExUuidCreate(&Attach);
			if (!NT_SUCCESS(ns))
			{
				LOG_DEBUG("ExUuidCreate ns = %x\r\n", ns);
				return nullptr;
			}
#if !defined(HIDDEN)
			RtlStringCchPrintfW(szGuid, MAX_PATH,
				L"\\Driver\\{%08x-%04x-%04x-%02x-%02x-%02x-%02x}",
				Attach.Data1,
				Attach.Data2,
				Attach.Data3,
				Attach.Data4[0],
				Attach.Data4[1],
				Attach.Data4[2],
				Attach.Data4[3]);
#else
			_swprintf(szGuid,
				L"\\Driver\\{%08x-%04x-%04x-%02x-%02x-%02x-%02x}",
				Attach.Data1,
				Attach.Data2,
				Attach.Data3,
				Attach.Data4[0],
				Attach.Data4[1],
				Attach.Data4[2],
				Attach.Data4[3]);
#endif

			UNICODE_STRING nsAttachName;
			RtlInitUnicodeString(&nsAttachName, szGuid);
			ns = IoCreateDriver(&nsAttachName, (PDRIVER_INITIALIZE)ddk::nt_drivers::new_driver_object);
			if (NT_SUCCESS(ns))
			{
				if (!m_drvobj_list.empty())
				{
					return m_drvobj_list.back();
				}
			}
			return nullptr;
		}
		static
			NTSTATUS NTAPI
			new_driver_object(
				IN PDRIVER_OBJECT driverObject,
				IN PUNICODE_STRING registryPath
			)
		{
			UNREFERENCED_PARAMETER(registryPath);
			ddk::nt_drivers::getInstance().add_driver_obj(driverObject);
			return STATUS_SUCCESS;
		}
		void add_driver_obj(PDRIVER_OBJECT drv_obj)
		{
			drv_obj->DriverUnload = nullptr;
			m_drvobj_list.push_back(drv_obj);
			InterlockedIncrement(&m_count);
		}
		void del_driver_obj(PDRIVER_OBJECT drv_obj)
		{
			_lock.acquire();
			if (drv_obj->DriverUnload)
			{
				drv_obj->DriverUnload(drv_obj);
			}
			IoDeleteDriver(drv_obj);
			//ObMakeTemporaryObject(drv_obj); 这样子删除时，有一定几率爆炸
			_lock.release();
		}
	private:
		LONG m_count;
		std::vector<PDRIVER_OBJECT>m_drvobj_list;
		nt_lock _lock;
	};
};
