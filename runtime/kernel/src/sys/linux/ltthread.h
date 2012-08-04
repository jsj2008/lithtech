// ------------------------------------------------------------------------- //
//
// FILE      : L T T H R E A D . H
//
// CREATED   : 11/05/99
//
// AUTHOR    : Matthew Scott
//
// COPYRIGHT : Monolith Productions Inc.
//
// ORIGN     : lithtech 1.5/2.0 lthread.h
//
// ------------------------------------------------------------------------- //

#ifndef __LINUXTHREAD_H__
#define __LINUXTHREAD_H__

// ------------------------------------------------------------------------- //
// Includes this module depends on
// ------------------------------------------------------------------------- //
#ifndef __STDLTERROR_H__
#include "stdlterror.h"
#endif

#ifndef __ITHREAD_H__
#include "ithread.h"
#endif

#ifndef __LTINTEGER_H__
#include "ltinteger.h"
#endif

#include <pthread.h>
#include <sys/time.h>
#include <errno.h>

// ------------------------------------------------------------------------- //
// Typedefs & Enums
// ------------------------------------------------------------------------- //

// ------------------------------------------------------------------------- //
// Class Definitions
// ------------------------------------------------------------------------- //

//
// CLASS: CSysThread
//
class CSysThread : public IThread {
	public:
		CSysThread();
		// implement the pure vitural interface defined in ithread.h
		virtual ESTDLTResults Run    (EThreadPriority pri = ThreadNormal);
		virtual ESTDLTResults Wakeup ();
		virtual ESTDLTResults Term   (bool blocking = true);
        virtual void          WaitForTerm();
	protected:
		pthread_t	m_ThreadID;
		pthread_cond_t	m_ThreadCond;
		pthread_mutex_t	m_ThreadMutex;
		pthread_mutex_t	m_WakeupMutex;
		uint32          m_WakeupCnt;

		bool	IsInThisThread() const { return pthread_self() == m_ThreadID; }
		static  void          	*ThreadLaunch	(void *pWhichThread);
		virtual void           	ThreadRun    	() = 0;
		virtual ESTDLTResults  	ThreadInit   	();
		virtual ESTDLTResults  	ThreadTerm   	();
		virtual void           	Sleep        	();
};

inline
CSysThread::CSysThread() {
	// m_ThreadCond		= PTHREAD_COND_INITIALIZER;
	// m_ThreadMutex 	= PTHREAD_MUTEX_INITIALIZER;
	pthread_cond_init (&m_ThreadCond, NULL);
	pthread_mutex_init (&m_ThreadMutex, NULL);
	pthread_mutex_init (&m_WakeupMutex, NULL);
	m_WakeupCnt = 0;
}


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
		pthread_mutex_t	m_Mutex;
};

inline
CSysSyncVar::CSysSyncVar() {
	pthread_mutex_init (&m_Mutex, NULL);
}

inline ESyncResult
CSysSyncVar::Begin() {
	pthread_mutex_lock (&m_Mutex);
}

inline ESyncResult
CSysSyncVar::End() {
	pthread_mutex_unlock (&m_Mutex);
}

inline ESyncResult
CSysSyncVar::Wait() {
	pthread_mutex_lock (&m_Mutex);
	pthread_mutex_unlock (&m_Mutex);
}

inline
CSysSyncVar::~CSysSyncVar() {
	pthread_mutex_destroy (&m_Mutex);
}


//
// CLASS: CSysSerialVar
//
class CSysSerialVar : public ISerialVar {
	public:
		CSysSerialVar();
		~CSysSerialVar();
		// implement this interface (see IThread.h)
		ESyncResult Lock();
		ESyncResult Unlock();
	protected:
		friend class CSysEventVar;
		pthread_mutex_t	m_Mutex;
		// attributes for mutex object
		pthread_mutexattr_t m_MutexAttr;
};

inline
CSysSerialVar::CSysSerialVar() 
{
	// allow a thread to call multiple times in a row
	pthread_mutexattr_init(&m_MutexAttr);
	pthread_mutexattr_settype(&m_MutexAttr, PTHREAD_MUTEX_RECURSIVE_NP);
	pthread_mutex_init(&m_Mutex, &m_MutexAttr);

	
}

inline
CSysSerialVar::~CSysSerialVar() 
{
	pthread_mutex_destroy(&m_Mutex);
	pthread_mutexattr_destroy(&m_MutexAttr);
}


inline ESyncResult
CSysSerialVar::Lock() 
{
	pthread_mutex_lock (&m_Mutex);
	return SYNC_OK;
}

inline ESyncResult
CSysSerialVar::Unlock() {
	pthread_mutex_unlock (&m_Mutex);
	return SYNC_OK;
}

//
// CLASS: CSysEventVar
//
class CSysEventVar : public IEventVar, public CSysSerialVar {
	public:
		CSysEventVar();
		// implement this interface (see IThread.h)
		ESyncResult Signal();
		ESyncResult WaitFor(uint32 timeout_ms =0);
		ESyncResult Lock() { return m_Mutex.Lock(); }
		ESyncResult Unlock() { return m_Mutex.Unlock(); }
	protected:
		CSysSerialVar m_Mutex;
		pthread_cond_t m_Cond;
};

inline
CSysEventVar::CSysEventVar() {
	pthread_cond_init (&m_Cond, NULL);
}

inline ESyncResult
CSysEventVar::Signal() {
	pthread_cond_signal(&m_Cond);
};

inline ESyncResult 
CSysEventVar::WaitFor(uint32 timeout_ms) {
//	if (timeout_ms)
//	{
		// get current time
		timeval now;
		gettimeofday(&now, NULL);

		// calculate absolute time timeout
		timespec abstime;
		abstime.tv_sec = now.tv_sec;
		abstime.tv_nsec = (now.tv_usec + timeout_ms * 1000) * 1000;

		switch (pthread_cond_timedwait(&m_Cond, &m_Mutex.m_Mutex, &abstime))
		{
			case 0:         return SYNC_OK;
			case ETIMEDOUT: return SYNC_TIMEOUT;
			default:        return SYNC_ERROR;
		}
//	}
//	else
//		return (pthread_cond_wait(&m_Cond, &m_Mutex.m_Mutex) == 0 ? SYNC_OK : SYNC_ERROR);
}

#endif  // __LINUXTHREAD_H__

