#pragma once
namespace ddk::callback
{
	class nt_cmreg_callback
	{
	public:
		using nt_cmreg_callback_type = std::function<NTSTATUS(PVOID, PVOID)>;
		nt_cmreg_callback() {
			_init = false;
			ExInitializeRundownProtection(&run_for);
		}
		~nt_cmreg_callback() {
			if (_init)
			{
				CmUnRegisterCallback(_registryCallbackCookie);
			}
			ExWaitForRundownProtectionRelease(&run_for);
			_callback.clear();
		}
	public:
		bool init(std::wstring _wsAltitude)
		{
			if (_init)
			{
				return true;
			}
			RtlInitUnicodeString(&_altitude, _wsAltitude.c_str());
			auto status = CmRegisterCallbackEx(
				ddk::callback::nt_cmreg_callback::_RegistryCallback,
				&_altitude,
				g_pDriverObject,
				this,
				&_registryCallbackCookie,
				NULL);
			if (!NT_SUCCESS(status))
			{
				LOG_DEBUG("failed CmRegisterCallbackEx\r\n");
				return false;
				//KernelStlRaiseException(KMODE_EXCEPTION_NOT_HANDLED);
			}
			_init = true;
			return true;
		}
	public:
		static NTSTATUS _RegistryCallback(
			_In_ PVOID CallbackContext,
			_In_opt_ PVOID Argument1,
			_In_opt_ PVOID Argument2)
		{
			if (CallbackContext)
			{
				auto pThis = reinterpret_cast<nt_cmreg_callback*>(CallbackContext);
				__try
				{
					return pThis->_do_Callback(
						Argument1,
						Argument2);
				}
				__except (1)
				{

				}
			}
			return STATUS_SUCCESS;
		}
		NTSTATUS _do_Callback(
			_In_opt_ PVOID Argument1,
			_In_opt_ PVOID Argument2)
		{
			NTSTATUS ns = STATUS_SUCCESS;
			ExAcquireRundownProtection(&run_for);
			for (auto _pfn : _callback)
			{
				ns = _pfn(
					Argument1,
					Argument2);
				if (ns == STATUS_CALLBACK_BYPASS)
				{
					break;
				}
				if (!NT_SUCCESS(ns))
				{
					break;
				}
			}
			ExReleaseRundownProtection(&run_for);
			return ns;
		}
		bool reg_callback(nt_cmreg_callback_type callback)
		{
			if (!_init)
			{
				//没有初始化，用默认值初始化
				if (!init(L"40000"))
				{
					return false;
				}
			}
			_callback.push_back(callback);
			return true;
		}
	private:
		bool _init;
		EX_RUNDOWN_REF run_for;
		LARGE_INTEGER _registryCallbackCookie;
		UNICODE_STRING _altitude;
		std::vector<nt_cmreg_callback_type>_callback;
	};
}