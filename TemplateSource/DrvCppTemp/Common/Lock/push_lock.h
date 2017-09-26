#pragma once
namespace ddk
{
	class push_lock
	{
	public:
		push_lock() {
			FltInitializePushLock(&_lock);
		}
		~push_lock() {
			FltDeletePushLock(&_lock);
		}
		void acqurie_read() {
			FltAcquirePushLockShared(&_lock);
		}
		void acqurie_write() {
			FltAcquirePushLockExclusive(&_lock);
		}
		void release() {
			FltReleasePushLock(&_lock);
		}
	private:
		EX_PUSH_LOCK _lock;
	};
};