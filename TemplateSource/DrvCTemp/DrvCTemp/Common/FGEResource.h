#include <ntddk.h>

typedef struct _FG_EResource_LOCK
{
	ERESOURCE m_lock;
}ELOCK;

/*初始化，在DriverEntry执行*/
VOID __stdcall initELock(ELOCK *lpLock);
/*卸载，在DriverUnload内执行*/
VOID __stdcall deleteELock(ELOCK *lpLock);
/*读锁*/
VOID __stdcall eLockRead(ELOCK *lpLock);
/*写锁*/
VOID __stdcall eLockWrite(ELOCK *lpLock);
/*读优先锁*/
VOID __stdcall eLockReadStarveWriter(ELOCK *lpLock);
/*写优先锁*/
VOID __stdcall eLockWriteStarveReader(ELOCK *lpLock);
/*解锁*/
VOID __stdcall eUnlock(ELOCK *lpLock);