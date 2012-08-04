// ------------------------------------------------------------------------- //
//
// FILE      : L T T H R E A D . C P P 
//
// CREATED   : 11/05/99
//
// AUTHOR    : Matthew Scott
//
// COPYRIGHT : Monolith Productions Inc.
//
// ORIGN     : 
//
// ------------------------------------------------------------------------- //

// ------------------------------------------------------------------------- //
// Includes this module depends on
// ------------------------------------------------------------------------- //
//#include "stdlttypes.h"
//#include "stdltdefs.h"
#include "stdlterror.h"

#include "ithread.h"
#include "ltthread.h"
#include <pthread.h>
#include <linux/errno.h>

// ------------------------------------------------------------------------- //
// 
// Implementation of public interface defined in ithread.h
// 
// ------------------------------------------------------------------------- //

// ------------------------------------------------------------------------- //
// NAME: Run
// 
// per thread / same thread as caller
//
// This is called on a an object derived from SysThread.  We are still running
// in the same thread as the caller, so create a new thread passing in the info
// about what object I am so that when I start in the new thread, I know who I 
// am
// ------------------------------------------------------------------------- //
ESTDLTResults CSysThread::Run (EThreadPriority pri )
{
	// create the thread
	if (pthread_create(&m_ThreadID, NULL, 
		&CSysThread::ThreadLaunch, (void *)this) != 0){
		CRITICAL_ERROR("CSysThreads::Run()", "Can't start thread\n");
	}
	return (STDLT_OK);
}

// ------------------------------------------------------------------------- //
// NAME: Wakeup
// 
// per thread / own thread
// ------------------------------------------------------------------------- //
ESTDLTResults CSysThread::Wakeup () 
{
	uint32 result;

	// aquire lock to serialize access to the counter var
	if (pthread_mutex_lock (&m_WakeupMutex) != 0) {
		CRITICAL_ERROR("CSysThreads::Wakeup()", 
			"critical error getting Wakeup mutex\n");
	}

	// one more wakeup call
	m_WakeupCnt++;
	
	// aquire lock to sync signalling thread to waiting thread
	if ((result = pthread_mutex_trylock (&m_ThreadMutex)) != 0) {
		// There was an error getting the lock.  This is fine
		// as long as it was EBUSY
		if (result != EBUSY) {
			CRITICAL_ERROR("CSysThreads::Wakeup()", 
				"critical error getting mutex\n");
		}
	} else {
		
		// We actually got the lock, so the other thread must be sleeping,
		// decrement the wakeup (that we are now servicing) and signal the
		// other thread
		m_WakeupCnt--;
		// ASSERT (m_WakeupCnt == 0);

		// Signal sleeping thread
		if (pthread_cond_signal (&m_ThreadCond) != 0) {
			CRITICAL_ERROR("CSysThreads::Wakeup()", 
				"critical error signalling cond\n");
		}

		// free lock to sync signalling thread to waiting thread
		if (pthread_mutex_unlock (&m_ThreadMutex) != 0) {
			CRITICAL_ERROR("CSysThreads::Wakeup()", 
				"critical error freeing mutex\n");
		}
	}

	// free lock to serialize access to the counter var
	if (pthread_mutex_unlock (&m_WakeupMutex) != 0) {
		CRITICAL_ERROR("CSysThreads::Wakeup()", 
			"critical error freeing Wakeup mutex\n");
	}
 	return (STDLT_OK);
}

// ------------------------------------------------------------------------- //
// NAME: Sleep
// 
// per thread / own thread
// ------------------------------------------------------------------------- //
void		  
CSysThread::Sleep  () {
	// aquire lock to serialize access to the counter var
	if (pthread_mutex_lock (&m_WakeupMutex) != 0) {
		CRITICAL_ERROR("CSysThreads::Wakeup()", 
			"critical error getting Wakeup mutex\n");
	}

	// someone called a wakeup while we were running, so don't go back to sleep
	if (m_WakeupCnt > 0) {
		// one less sleep call
		m_WakeupCnt--;

		// free lock to serialize access to the counter var
		if (pthread_mutex_unlock (&m_WakeupMutex) != 0) {
			CRITICAL_ERROR("CSysThreads::Wakeup()", 
				"critical error freeing Wakeup mutex\n");
		}
	} else {
		// free lock to serialize access to the counter var
		if (pthread_mutex_unlock (&m_WakeupMutex) != 0) {
			CRITICAL_ERROR("CSysThreads::Wakeup()", 
				"critical error freeing Wakeup mutex\n");
		}
		// wait for a signal
		if (pthread_cond_wait (&m_ThreadCond, &m_ThreadMutex) != 0) {
			CRITICAL_ERROR("CSysThreads::Sleep()", 
				"critical error waiting on cond\n");
		}
	}
}

// ------------------------------------------------------------------------- //
// NAME: Term
// 
// per thread / own thread
// ------------------------------------------------------------------------- //
ESTDLTResults 
CSysThread::Term   (bool blocking) 
{
	return (STDLT_OK);
}

// ------------------------------------------------------------------------- //
// NAME: WaitForTerm
// 
// per thread / any other thread
// ------------------------------------------------------------------------- //
void
CSysThread::WaitForTerm() {
	pthread_join(m_ThreadID, NULL);	
}

// ------------------------------------------------------------------------- //
// 
// Implementation of private interface defined in systhread.h
// 
// ------------------------------------------------------------------------- //

// ------------------------------------------------------------------------- //
// NAME: ThreadLaunch
// 
// static (only one) / own thread
// 
// This is the static launcher proc that is now running in the new thread, but
// is the same launcher for all threads.  So it needs to pass control to the
// ThreadRun method of the particular object that had its Run method called
// ------------------------------------------------------------------------- //
void *
CSysThread::ThreadLaunch (void *pWhichThread) {
	((CSysThread*)pWhichThread)->ThreadInit();
	((CSysThread*)pWhichThread)->ThreadRun();
	((CSysThread*)pWhichThread)->ThreadTerm();
	return 0;
}

// ------------------------------------------------------------------------- //
// NAME: ThreadInit
// 
// per thread / own thread
// ------------------------------------------------------------------------- //
ESTDLTResults
CSysThread::ThreadInit () {
	// Anything that needs to be done before control is passed to the 
	// thread main loop (ThreadRun)

	// lock the mutex that is being used (with ThreadCond) to implement Sleep
	// and Wakeup
	if (pthread_mutex_lock(&m_ThreadMutex) != 0) {
		CRITICAL_ERROR("CSysThreads::ThreadInit",
			"Critical error locking mutex\n");
	}
	return (STDLT_OK);
}

// ------------------------------------------------------------------------- //
// NAME: ThreadTerm
// 
// per thread / own thread
// ------------------------------------------------------------------------- //
#include <cstdio>
ESTDLTResults
CSysThread::ThreadTerm () {
	// Anything that needs to be done before this thread is completely
	// destroyed
	pthread_exit(NULL);
	return (STDLT_OK);
}

