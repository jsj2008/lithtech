// ------------------------------------------------------------------------- //
//
// FILE      : L T T H R E A D . H
//
// CREATED   : 11/17/00
//
// AUTHOR    : Rick Lambright (based on Matt Scott's PS2 version)
//
// COPYRIGHT : LithTech Inc.  All Rights Reserved
//
// ORIGN     : lithtech 2.1 ltthread.h
//
// ------------------------------------------------------------------------- //

#ifndef __LTTHREAD_H__
#define __LTTHREAD_H__

#ifndef __STDLTERROR_H__
#include "stdlterror.h"
#endif

#ifndef __ITHREAD_H__
#include "ithread.h"
#endif

#define WIN32_LEAN_AND_MEAN
#ifndef __WINDOWS_H__
#include <windows.h>  // ecch (needed for inline critical sections)
#define __WINDOWS_H__
#endif

// ------------------------------------------------------------------------- //
// Defines
// ------------------------------------------------------------------------- //
#define THREAD_STACK_SIZE 65536 // 64K thread stacks

// ------------------------------------------------------------------------- //
// Typedefs & Enums
// ------------------------------------------------------------------------- //

// ------------------------------------------------------------------------- //
// Class Definitions
// ------------------------------------------------------------------------- //

//
// CLASS: CSysSyncVar
//
class CSysSyncVar : public ISyncVar {
    public:
        CSysSyncVar();
        ~CSysSyncVar();
        // implement this interface (see IThread.h)
        ESyncResult Begin();
        ESyncResult End();
        ESyncResult Wait();
    protected:
        HANDLE m_hMutex;
}; // CSysSyncVar

inline
CSysSyncVar::CSysSyncVar() 
{
    m_hMutex = CreateMutex(NULL, FALSE, NULL);
}

inline
CSysSyncVar::~CSysSyncVar() 
{
    if (m_hMutex != NULL)
        CloseHandle(m_hMutex);
}

inline ESyncResult
CSysSyncVar::Begin() 
{
    return (WaitForSingleObject(m_hMutex, INFINITE) == WAIT_FAILED ? SYNC_ERROR : SYNC_OK);
}

inline ESyncResult
CSysSyncVar::End() 
{
    return (ReleaseMutex(m_hMutex) ? SYNC_OK : SYNC_ERROR);
}

inline ESyncResult
CSysSyncVar::Wait() 
{
    if (WaitForSingleObject(m_hMutex, INFINITE) != WAIT_FAILED)
        return (ReleaseMutex(m_hMutex) ? SYNC_OK : SYNC_ERROR);
    return(SYNC_ERROR);
}

//
// CLASS: CSysSerialVar
//
class CSysSerialVar : public ISerialVar 
{
    public:
        CSysSerialVar();
        ~CSysSerialVar();
        // implement this interface (see IThread.h)
        ESyncResult Lock();
        ESyncResult Unlock();
    protected:
        CRITICAL_SECTION m_CritSect;
};

inline
CSysSerialVar::CSysSerialVar() 
{
    InitializeCriticalSection(&m_CritSect);
}

inline
CSysSerialVar::~CSysSerialVar() 
{
    DeleteCriticalSection(&m_CritSect);
}

inline ESyncResult
CSysSerialVar::Lock() 
{
    EnterCriticalSection(&m_CritSect);
    return(SYNC_OK);
}

inline ESyncResult
CSysSerialVar::Unlock() 
{
    LeaveCriticalSection(&m_CritSect);
    return(SYNC_OK);
}

//
// CLASS: CSysEventVar
//
class CSysEventVar : public IEventVar {
	public:
		CSysEventVar();
		// implement this interface (see IThread.h)
		ESyncResult Signal();
		ESyncResult WaitFor(uint32 timeout_ms =INFINITE);
		ESyncResult Lock() { return m_Mutex.Lock(); }
		ESyncResult Unlock() { return m_Mutex.Unlock(); }
	protected:
		CSysSerialVar m_Mutex;
		HANDLE m_hEvent;
};

inline
CSysEventVar::CSysEventVar() {
	m_hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
}

inline ESyncResult
CSysEventVar::Signal() {
	return (::SetEvent(m_hEvent) ? SYNC_OK : SYNC_ERROR);
};

inline ESyncResult 
CSysEventVar::WaitFor(uint32 timeout_ms) {
	m_Mutex.Unlock();
	switch (WaitForSingleObject(m_hEvent, timeout_ms))
	{
		case WAIT_OBJECT_0:
			m_Mutex.Lock(); // Lock on success to duplicate pthread_cond_timedwait
			return SYNC_OK;
		case WAIT_TIMEOUT:
			m_Mutex.Lock(); // Lock on timeout to duplicate pthread_cond_timedwait
			return SYNC_TIMEOUT;
		default: 
			return SYNC_ERROR;
	}
}

//
// CLASS: CSysThread
//
class CSysThread : public IThread 
{
    public:
        CSysThread();
        // implement the pure vitural interface defined in ithread.h
        virtual ESTDLTResults Run(EThreadPriority pri = ThreadNormal);
        virtual ESTDLTResults Wakeup();
        virtual ESTDLTResults Term(bool blocking = true); // avoid using this function if possible!
        virtual void          WaitForTerm();
    protected:
        // these methods are not in IThread
        bool    IsInThisThread() const;
        bool    IsThreadActive() const;
        static  unsigned int __stdcall  ThreadLaunch(void * pWhichThread);
        virtual void                    ThreadRun() = 0;
        virtual ESTDLTResults           ThreadInit();
        virtual ESTDLTResults           ThreadTerm();
        virtual void                    Sleep();
    protected:
        // not portable
        int     LTPriToWinPri(EThreadPriority pri);
        HANDLE  GetWinThreadHandle() { return m_hThread; }
        DWORD   GetWinThreadId() { return m_ThreadID; }
    private:
        // data members
        CSysSerialVar m_Lock; // thread-safe access to m_LocalWakeup
        int m_LocalWakeup;
        HANDLE m_hThread;
        unsigned int m_ThreadID;
}; // CSysThread

inline
CSysThread::CSysThread() 
  : m_LocalWakeup(0),
    m_hThread(NULL),
    m_ThreadID(0)
{
    m_LocalWakeup=0;
} // CSysThread constructor

inline int
CSysThread::LTPriToWinPri(EThreadPriority pri) 
{
    if (pri == ThreadLow)  // aka ThreadPri0
        return THREAD_PRIORITY_LOWEST;
    else if (pri == ThreadPri1)
        return THREAD_PRIORITY_BELOW_NORMAL;
    else if (pri == ThreadNormal) // aka ThreadPri2
        return THREAD_PRIORITY_NORMAL;
    else if (pri == ThreadPri3)
        return THREAD_PRIORITY_ABOVE_NORMAL;
    return THREAD_PRIORITY_HIGHEST;
}

inline bool 
CSysThread::IsInThisThread() const 
{ 
    return m_ThreadID && GetCurrentThreadId() == m_ThreadID;
} // CSysThread::IsInThisThread

inline bool 
CSysThread::IsThreadActive() const
{ 
    return m_hThread != NULL && 
           WaitForSingleObject(m_hThread, 0) == WAIT_TIMEOUT;
} // CSysThread::IsThreadActive

#endif  // __WIN_LTTHREAD_H__