#pragma once
namespace ddk::callback
{
	class nt_processor_callback
	{
	public:
		using pfn_processor_callback = std::function<void(PKE_PROCESSOR_CHANGE_NOTIFY_CONTEXT, PNTSTATUS)>;
		nt_processor_callback() {
			init();
		}
		~nt_processor_callback() {
			if (_processor_cb)
			{
				KeDeregisterProcessorChangeCallback(_processor_cb);
			}
		}
	private:
		void init()
		{
			_callbackPfn = nullptr;
			_processor_cb = nullptr;
			_processor_cb = KeRegisterProcessorChangeCallback(ddk::callback::nt_processor_callback::ProcessorCallback, this, 0);
		}
	public:
		static VOID
			ProcessorCallback(
				__in PVOID CallbackContext,
				__in PKE_PROCESSOR_CHANGE_NOTIFY_CONTEXT ChangeContext,
				__inout PNTSTATUS OperationStatus
			)
		{
			auto pThis = reinterpret_cast<ddk::callback::nt_processor_callback*>(CallbackContext);
			if (pThis)
			{
				pThis->OnCallBack(ChangeContext, OperationStatus);
			}
		}
	public:
		VOID OnCallBack(
			__in PKE_PROCESSOR_CHANGE_NOTIFY_CONTEXT ChangeContext,
			__inout PNTSTATUS OperationStatus
		)
		{
			if (_callbackPfn)
			{
				_callbackPfn(ChangeContext, OperationStatus);
			}
		}
	public:
		bool set_callback(pfn_processor_callback _callback)
		{
			if (_processor_cb)
			{
				_callbackPfn = _callback;
				return true;
			}
			return false;
		}
	private:
		pfn_processor_callback _callbackPfn;
		PVOID _processor_cb;
	};
};
