#pragma once

namespace ddk
{
	class nt_kevent
	{
	public:
		nt_kevent() {
			KeInitializeEvent(&_Event, SynchronizationEvent, FALSE);
		}
		~nt_kevent(){}
	private:
		KEVENT _Event;
	public:
		void wait(){
			KeWaitForSingleObject(&_Event, Executive, KernelMode, FALSE, 0);
		}
		void set(){
			KeSetEvent(&_Event, 0, FALSE);
		}
	};
}