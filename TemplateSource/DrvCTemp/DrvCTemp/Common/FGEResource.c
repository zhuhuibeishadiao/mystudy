#include "FGEResource.h"

VOID __stdcall initELock(ELOCK *lpLock)
{
	ExInitializeResourceLite(&lpLock->m_lock);
}

VOID __stdcall deleteELock(ELOCK *lpLock)
{
	ExDeleteResourceLite(&lpLock->m_lock);
}

VOID __stdcall eLockRead(ELOCK *lpLock)
{
	KeEnterCriticalRegion();
	ExAcquireResourceSharedLite(&lpLock->m_lock, TRUE);
}

VOID __stdcall eLockWrite(ELOCK *lpLock)
{
	KeEnterCriticalRegion();
	ExAcquireResourceExclusiveLite(&lpLock->m_lock, TRUE);
}

VOID __stdcall eLockReadStarveWriter(ELOCK *lpLock)
{
	KeEnterCriticalRegion();
	ExAcquireSharedStarveExclusive(&lpLock->m_lock, TRUE);
}

VOID __stdcall eLockWriteStarveReader(ELOCK *lpLock)
{
	KeEnterCriticalRegion();
	ExAcquireSharedWaitForExclusive(&lpLock->m_lock, TRUE);
}

VOID __stdcall eUnlock(ELOCK *lpLock)
{
	ExReleaseResourceLite(&lpLock->m_lock);
	KeLeaveCriticalRegion();
}