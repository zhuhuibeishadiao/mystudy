#pragma once
namespace ddk
{
	class nt_rundown_protect
	{
	public:
		nt_rundown_protect() {
			ExInitializeRundownProtection(&run_for);
		};
		~nt_rundown_protect() {
			ExWaitForRundownProtectionRelease(&run_for);
		};
	public:
		void acquire() {
			ExAcquireRundownProtection(&run_for);
		}
		void release() {
			ExReleaseRundownProtection(&run_for);
		}
		void wait() {
			ExWaitForRundownProtectionRelease(&run_for);
		}
	private:
		EX_RUNDOWN_REF run_for;
	};
}