#pragma once
namespace ddk::callback
{
	enum nt_pnp_callback_class
	{
		DISK = 1,
		CDROM,
		VOLUME,
		PARTITION,
		ALL,
	};
	class nt_pnp_callback {
	public:
		using nt_pnp_callback_type = std::function<NTSTATUS(PVOID)>;
		nt_pnp_callback() {
			_NotificationEntry = nullptr;
		}
		~nt_pnp_callback() {
			if (_NotificationEntry)
			{
				IoUnregisterPlugPlayNotification(_NotificationEntry);
			}
		}
		nt_pnp_callback(nt_pnp_callback_class _fclass)
		{
			_NotificationEntry = nullptr;
			create_callback(_fclass);
		}
	public:
		bool set_callback(nt_pnp_callback_type _function)
		{
			if (!_NotificationEntry)
			{
				return false;
			}
			_callbacks.push_back(_function);
			return true;
		}
	public:
		bool create_callback(nt_pnp_callback_class _fclass)
		{
			NTSTATUS ns = STATUS_UNSUCCESSFUL;
			this->_class = _fclass;
			switch (_class)
			{
			case ddk::callback::nt_pnp_callback_class::VOLUME:
				ns = IoRegisterPlugPlayNotification(
					EventCategoryDeviceInterfaceChange,
					PNPNOTIFY_DEVICE_INTERFACE_INCLUDE_EXISTING_INTERFACES,//0 Magic
					(PVOID)(&GUID_DEVINTERFACE_VOLUME),
					g_pDriverObject,
					(PDRIVER_NOTIFICATION_CALLBACK_ROUTINE)nt_pnp_callback::DriverDevInterxNotifyCallBack,
					this,
					&_NotificationEntry);
				break;
			case ddk::callback::nt_pnp_callback_class::PARTITION:
				ns = IoRegisterPlugPlayNotification(
					EventCategoryDeviceInterfaceChange,
					PNPNOTIFY_DEVICE_INTERFACE_INCLUDE_EXISTING_INTERFACES,//0 Magic
					(PVOID)(&GUID_DEVINTERFACE_PARTITION),
					g_pDriverObject,
					(PDRIVER_NOTIFICATION_CALLBACK_ROUTINE)nt_pnp_callback::DriverDevInterxNotifyCallBack,
					this,
					&_NotificationEntry);
				break;
			case ddk::callback::nt_pnp_callback_class::ALL:
				ns = IoRegisterPlugPlayNotification(
					EventCategoryHardwareProfileChange,
					PNPNOTIFY_DEVICE_INTERFACE_INCLUDE_EXISTING_INTERFACES,//0 Magic
					nullptr,
					g_pDriverObject,
					(PDRIVER_NOTIFICATION_CALLBACK_ROUTINE)nt_pnp_callback::DriverDevInterxNotifyCallBack,
					this,
					&_NotificationEntry);
				break;
			case ddk::callback::nt_pnp_callback_class::DISK:
				ns = IoRegisterPlugPlayNotification(
					EventCategoryDeviceInterfaceChange,
					PNPNOTIFY_DEVICE_INTERFACE_INCLUDE_EXISTING_INTERFACES,//0 Magic
					(PVOID)(&GUID_DEVINTERFACE_DISK),
					g_pDriverObject,
					(PDRIVER_NOTIFICATION_CALLBACK_ROUTINE)nt_pnp_callback::DriverDevInterxNotifyCallBack,
					this,
					&_NotificationEntry);
				break;
			case ddk::callback::nt_pnp_callback_class::CDROM:
				ns = IoRegisterPlugPlayNotification(
					EventCategoryDeviceInterfaceChange,
					PNPNOTIFY_DEVICE_INTERFACE_INCLUDE_EXISTING_INTERFACES,
					(PVOID)(&GUID_DEVINTERFACE_CDROM),
					g_pDriverObject,
					(PDRIVER_NOTIFICATION_CALLBACK_ROUTINE)nt_pnp_callback::DriverDevInterxNotifyCallBack,
					this,
					&_NotificationEntry);
				break;
			default:
				break;
			}
			if (!NT_SUCCESS(ns))
			{
				return false;
			}
			return true;
		}
		static NTSTATUS NTAPI DriverDevInterxNotifyCallBack(
			IN PVOID NotificationStructure,
			IN PVOID Context)
		{
			auto pThis = reinterpret_cast<nt_pnp_callback*>(Context);
			__try
			{
				return pThis->do_callback(NotificationStructure);
			}
			__except (1)
			{

			}
			return STATUS_SUCCESS;
		}
		NTSTATUS do_callback(PVOID NotificationStructure)
		{
			for (auto _pfn : _callbacks)
			{
				auto ns = _pfn(NotificationStructure);
				if (!NT_SUCCESS(ns))
				{
					return ns;
				}
			}
			return STATUS_SUCCESS;
		}
		nt_pnp_callback & operator = (nt_pnp_callback &_pnp)
		{
			this->_class = _pnp.get_class();
			_pnp.get_callbacks(this->_callbacks);
			if (_pnp.get_entry())
			{
				this->create_callback(this->_class);
			}
			_pnp.clear();
			return (*this);
		}
		PVOID get_entry() {
			return _NotificationEntry;
		}
		nt_pnp_callback_class get_class() {
			return _class;
		}
		void clear()
		{
			if (_NotificationEntry)
			{
				IoUnregisterPlugPlayNotification(_NotificationEntry);
			}
			_NotificationEntry = nullptr;
			_callbacks.clear();
		}
		void get_callbacks(std::vector<nt_pnp_callback_type> &_new_callbacks)
		{
			_new_callbacks = _callbacks;
		}
	private:
		nt_pnp_callback_class _class;
		PVOID _NotificationEntry;
		std::vector<nt_pnp_callback_type>_callbacks;
	};
};