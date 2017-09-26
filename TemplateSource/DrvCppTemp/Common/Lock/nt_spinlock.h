#pragma once
namespace ddk
{
	class spinlock_mutex
	{
		std::atomic_flag flag;
	public:
		spinlock_mutex(){
			flag._My_flag = 0;
		}
		void lock()
		{
			while (flag.test_and_set(std::memory_order_acquire))
			{
				YieldProcessor();
			}
		}
		void unlock()
		{
			flag.clear(std::memory_order_release);
		}
	};

	class nt_spinlock
	{
	public:
		nt_spinlock()
		{
			spinlock = 0;
			KeInitializeSpinLock(&spinlock);
		}
		~nt_spinlock()
		{

		}
		void lock(PKLOCK_QUEUE_HANDLE handle)
		{
			if (KeGetCurrentIrql() < DISPATCH_LEVEL)
				KeAcquireInStackQueuedSpinLock(&spinlock, handle);
			else
				KeAcquireInStackQueuedSpinLockAtDpcLevel(&spinlock, handle);
		}
		void unlock(PKLOCK_QUEUE_HANDLE handle)
		{
			if (handle->OldIrql < DISPATCH_LEVEL)
				KeReleaseInStackQueuedSpinLock(handle);
			else
				KeReleaseInStackQueuedSpinLockFromDpcLevel(handle);
		}
	private:
		KSPIN_LOCK spinlock;
	};
};