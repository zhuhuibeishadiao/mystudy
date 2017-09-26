#pragma once
namespace ddk
{
	class nt_rwlock
	{
	public:
		nt_rwlock()
		{
			res = nullptr;
			res = reinterpret_cast<PERESOURCE>(malloc(PAGE_SIZE));
			NT_ASSERT(res != nullptr);
			NT_ASSERT(KeGetCurrentIrql() <= DISPATCH_LEVEL);
			res_ns = ExInitializeResourceLite(res);
		}
		~nt_rwlock()
		{
			NT_ASSERT(res != nullptr);
			NT_ASSERT(res_ns == STATUS_SUCCESS);
			ExDeleteResourceLite(res);
			free(res);
		}
		void lock_for_read()
		{
			NT_ASSERT(res != nullptr);
			NT_ASSERT(res_ns == STATUS_SUCCESS);
			NT_ASSERT(KeGetCurrentIrql() <= APC_LEVEL);
			ExEnterCriticalRegionAndAcquireResourceShared(res);
		}
		void lock_for_write()
		{
			NT_ASSERT(res != nullptr);
			NT_ASSERT(res_ns == STATUS_SUCCESS);
			NT_ASSERT(KeGetCurrentIrql() <= APC_LEVEL);
			ExEnterCriticalRegionAndAcquireResourceExclusive(res);
		}
		void unlock()
		{
			NT_ASSERT(res != nullptr);
			NT_ASSERT(res_ns == STATUS_SUCCESS);
			ExReleaseResourceAndLeaveCriticalRegion(res);
		}
	private:
		PERESOURCE res;
		NTSTATUS res_ns;
	};
};