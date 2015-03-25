#ifndef DX_LOCK_H
#define DX_LOCK_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "VOS_API/DX_VOS_BaseTypes.h"
#define DX_LOCK_TIMEOUT     10000   // If the lock is not locked for more than 10 
typedef struct _DxLock *DxLock;

DxStatus DxLock_Create(DxLock* lock);

void DxLock_Destroy(DxLock* lock);

DxStatus DxLock_Lock(DxLock lock, DxUint32 timeout);

void DxLock_Unlock(DxLock lock);

#ifdef __cplusplus
}
#endif

#endif
