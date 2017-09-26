#pragma once
namespace ddk
{
	class nt_ipi
	{
	public:
		template<class _Fn,
			class... _Args,
			class = typename std::enable_if<
			!std::is_same<typename std::decay<_Fn>::type, nt_ipi>::value>::type>
			explicit nt_ipi(_Fn&& _Fx, _Args&&... _Ax)
		{
			m_function = std::bind(std::forward<_Fn>(_Fx), std::forward<_Args>(_Ax)...);
			if (KeNumberProcessors > 1)
			{
				KeIpiGenericCall(ddk::nt_ipi::IpiCallBack, this);
			}
			else
			{
				Execute();
			}
		}
		static ULONG_PTR
			IpiCallBack(
				_In_ ULONG_PTR Argument
			)
		{
			auto _ipi = reinterpret_cast<nt_ipi*>(Argument);
			if (_ipi)
			{
				_ipi->Execute();
			}
			return 0;
		}
		void Execute()
		{
			m_function();
		}
	private:
		std::function<void()>m_function;
	};
}