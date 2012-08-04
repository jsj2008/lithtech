#ifndef __LTMUTEX_H__
#define __LTMUTEX_H__
//
// ltmutex.h - miscellaneous shared and portable declarations related to
// synchronization and mutexes.
//
// Copyright (C) 2000, LithTech Inc.  All Rights Reserved.
//

#ifndef __LTBASETYPES_H__
#include "ltbasetypes.h"
#endif

#ifndef __SYSTHREAD_H__
#include "systhread.h"
#endif

class CLTMutexSync
{
public:
    CLTMutexSync(CSysSerialVar& lockRef) 
      : m_lockRef(lockRef) { lockRef.Lock(); }
    ~CLTMutexSync() 
      { m_lockRef.Unlock(); }
private:
    CSysSerialVar& m_lockRef;   
}; // CLTMutexSync

// Note: declare LTSMP_THREAD_MUTEX in a PROTECTED block in your class
typedef CSysSerialVar LTSMPThreadMutex;
#define DECLARE_LTSMP_THREAD_MUTEX  mutable LTSMPThreadMutex m_lock;
#define LTSMP_SYNCHRONIZE CLTMutexSync ___ltsmp_lock_sync(m_lock);

#endif //  __LTMUTEX_H__



