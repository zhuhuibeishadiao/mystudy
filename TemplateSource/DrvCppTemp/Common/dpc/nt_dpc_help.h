#pragma once
namespace ddk
{
	class dpc_helper :public Singleton<dpc_helper>
	{
	public:
		using _dpc_callback = std::function<void()>;
		using _dpc_context = struct
		{
			_dpc_callback _callbackPfn;
		};
		static VOID
			DpcHelperRoutine(
				_In_ struct _KDPC *Dpc,
				_In_opt_ PVOID DeferredContext,
				_In_opt_ PVOID SystemArgument1,
				_In_opt_ PVOID SystemArgument2
			)
		{
			UNREFERENCED_PARAMETER(Dpc);
			UNREFERENCED_PARAMETER(SystemArgument1);
			UNREFERENCED_PARAMETER(SystemArgument2);

			auto pContext = reinterpret_cast<_dpc_context*>(DeferredContext);
			pContext->_callbackPfn();
			KeSignalCallDpcSynchronize(SystemArgument2);
			KeSignalCallDpcDone(SystemArgument1);
		}
		template<class _Fn,
			class... _Args>
		void CallDpcRoutine(_Fn&& _Fx, _Args&&... _Ax)
		{
			_dpc_context context = {};
			context._callbackPfn = std::bind(std::forward<_Fn>(_Fx), std::forward<_Args>(_Ax)...);
			KeGenericCallDpc(ddk::DpcHelper::DpcHelperRoutine, &context);
		}
	};
};