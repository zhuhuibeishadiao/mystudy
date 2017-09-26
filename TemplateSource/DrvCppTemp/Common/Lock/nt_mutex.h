#pragma once
namespace ddk
{
	class nt_mutex
	{
	public:
		nt_mutex()
		{
			KeInitializeGuardedMutex(&mutex);
		}
		~nt_mutex()
		{

		}
		void lock()
		{
			NT_ASSERT(KeGetCurrentIrql() <= APC_LEVEL);
			KeAcquireGuardedMutex(&mutex);
		}
		void unlock()
		{
			KeReleaseGuardedMutex(&mutex);
		}
		bool try_lock()
		{
			NT_ASSERT(KeGetCurrentIrql() <= APC_LEVEL);
			if (!KeTryToAcquireGuardedMutex(&mutex)) return false;
			return true;
		}
	private:
		KGUARDED_MUTEX mutex;
	};
};