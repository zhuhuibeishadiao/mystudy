#pragma once
namespace ddk
{
	VOID _launch_callback_timer(
		PKDPC Dpc,
		PVOID DeferredContext,
		PVOID SystemArgument1,
		PVOID SystemArgument2);
	class nt_timer
	{
	public:
		nt_timer()
		{
			ltime = 0;
			ltimes = 0;
			p_dd = false;
		}
		template<class _Fn,
			class... _Args,
			class = typename std::enable_if<
			!std::is_same<typename std::decay<_Fn>::type, nt_timer>::value>::type>
			explicit nt_timer(LONGLONG timer_time, LONG timer_times, _Fn&& _Fx, _Args&&... _Ax)
		{
			ltime = -timer_time;
			ltimes = timer_times;
			p_dd = true;
			if (timer_times)
				KeInitializeTimerEx(&ltimer, SynchronizationTimer);
			else
				KeInitializeTimer(&ltimer);
			m_function = std::bind(std::forward<_Fn>(_Fx), std::forward<_Args>(_Ax)...);

			KeInitializeDpc(&_dpc, ddk::_launch_callback_timer, this);
			_timer.QuadPart = this->ltime;
			if (this->ltimes != 0)
			{
				_timer.QuadPart = 0;
				KeSetTimerEx(&this->ltimer, _timer, this->ltimes, &_dpc);
			}
			else
			{
				KeSetTimer(&this->ltimer, _timer, &_dpc);
			}
		}
		nt_timer & operator = (nt_timer &timer_)
		{
			this->p_dd = timer_.p_dd;
			this->ltime = timer_.ltime;
			this->ltimes = timer_.ltimes;
			this->m_function = timer_.m_function;
			timer_.set_rel();

			if (this->ltimes)
				KeInitializeTimerEx(&this->ltimer, SynchronizationTimer);
			else
				KeInitializeTimer(&this->ltimer);
			KeInitializeDpc(&this->_dpc, ddk::_launch_callback_timer, this);
			this->_timer.QuadPart = this->ltime;
			if (this->ltimes != 0)
			{
				this->_timer.QuadPart = 0;
				KeSetTimerEx(&this->ltimer, this->_timer, this->ltimes, &this->_dpc);
			}
			else
			{
				KeSetTimer(&this->ltimer, this->_timer, &this->_dpc);
			}

			return (*this);
		}
		~nt_timer()
		{
			if (!p_dd)
				return;
			LOG_DEBUG("Begin Timer Release\r\n");
			KeSetTimer(&this->ltimer, _timer, NULL);
			KeCancelTimer(&ltimer);
			if (this->ltimes != 0)
			{
				KeFlushQueuedDpcs();
			}
			LOG_DEBUG("Release Timer\r\n");
		}
		void set_rel()
		{
			p_dd = false;
			KeSetTimer(&this->ltimer, _timer, NULL);
			KeCancelTimer(&ltimer);
			if (this->ltimes != 0)
			{
				KeFlushQueuedDpcs();
			}
		}
		void timer_function()
		{
			m_function();
			if (this->ltimes != 0)
			{
				return;
			}
			_timer.QuadPart = this->ltime;
			KeSetTimer(&this->ltimer, _timer, &_dpc);
		}
		LARGE_INTEGER _timer;
		KDPC _dpc;
		LONGLONG ltime;
		LONG ltimes;
		KTIMER ltimer;
		std::function<void()>m_function;
		bool p_dd;
	};
};