// ------------------------------------------------------------------------- //
//
// FILE      : L T T H R E A D . C P P
//
// CREATED   : 11/17/00
//
// AUTHOR    : Rick Lambright (based on Matt Scott's PS2 version)
//
// COPYRIGHT : LithTech Inc.  All Rights Reserved
//
// ORIGN     : lithtech 2.1 ltthread.cpp
//
// ------------------------------------------------------------------------- //

// ------------------------------------------------------------------------- //
// Includes this module depends on
// ------------------------------------------------------------------------- //
#include "bdefs.h"
#include "stdlterror.h"

#include "ithread.h"
#include "ltthread.h"

#include <process.h> // _beginthreadex etc.

// ------------------------------------------------------------------------- //
//
// Implementation of public interface defined in ithread.h
//
// ------------------------------------------------------------------------- //

// ------------------------------------------------------------------------- //
// NAME: Run
//
// per thread / same thread as caller - initiate a new thread
//
// ------------------------------------------------------------------------- //
ESTDLTResults
CSysThread::Run(
	EThreadPriority pri
	) 
{
	m_hThread = (HANDLE)_beginthreadex(
							NULL,						// security
							THREAD_STACK_SIZE,			// stack size
							&CSysThread::ThreadLaunch,  // start address
							(void *)this,				// arg list
							CREATE_SUSPENDED,			// init flag (0 == running, CREATE_SUSPENDED == suspended)
							&m_ThreadID);
	if (m_hThread != 0)
	{
		//
		// We create each thread suspended in case the current thread
		// is at very low priority and we might get starved before 
		// we have a chance to set it's priority.
		//
		if (pri != ThreadNormal)
			SetThreadPriority(m_hThread, LTPriToWinPri(pri));
		//
		// Off to the races...
		//
		if (ResumeThread(m_hThread) == 0xffffffff)
		{
			CRITICAL_ERROR ("CSysThread::Run","Can't resume thread\n");
			return STDLT_ERROR;
		}
		return STDLT_OK;
	}

	CRITICAL_ERROR("CSysThread::Run","Can't create thread\n");
	return STDLT_ERROR;
} // CSysThread::Run


// ------------------------------------------------------------------------- //
// NAME: Wakeup
//
// per thread / own thread
// ------------------------------------------------------------------------- //
ESTDLTResults
CSysThread::Wakeup() 
{
	bool bResume = true;

	m_Lock.Lock();
	if (IsInThisThread())
	{
		m_LocalWakeup++;
		bResume = false;
	}
	m_Lock.Unlock();

	if (bResume)
		return (ResumeThread(m_hThread) == 0xffffffff ? STDLT_ERROR : STDLT_OK);

	return (STDLT_OK);
}

// ------------------------------------------------------------------------- //
// NAME: Sleep
//
// per thread / own thread
// ------------------------------------------------------------------------- //
void
CSysThread::Sleep() 
{
	bool bSuspend = true;

	m_Lock.Lock();
	if (m_LocalWakeup > 0)
	{
		m_LocalWakeup--;
		bSuspend = false;
	}
	m_Lock.Unlock(); // unlock NOW - in case suspending self
	
	if (bSuspend)
		SuspendThread(m_hThread);
}

// ------------------------------------------------------------------------- //
// NAME: Term - terminates thread (A DANGEROUS THING TO DO on WIN32!!!)
//
// per thread / own thread
// ------------------------------------------------------------------------- //
ESTDLTResults
CSysThread::Term(
	bool blocking
	) 
{
	if (m_hThread != NULL)
	{

		bool bRemoteRequest = !IsInThisThread();
		if (bRemoteRequest)
		{
			//
			// test to see if still active under lock
			// (this is an effort to eliminate multiple calls to TerminateThread
			// which is a particularly nasty function.)
			//

			m_Lock.Lock(); // we want to serialize both the "is active" test
						   // _and_ the TerminateThread call
			if (IsThreadActive())
			{
				// handle is good and thread is not signaled - so must still be active
				if (TerminateThread(m_hThread, 0))
				{
					m_Lock.Unlock();
					if (blocking)
						WaitForTerm();
				}
				else
				{
					// TerminateThread failed - hmmm... could be that somebody
					// else killed the thread (or it went away itself) just before
					// we tried - or some kind of error happened.  Nothing else
					// to do now anyway.
					m_Lock.Unlock();
				}
			}
			else
			{
				// handle is bad or thread is signaled - either thread
				// was never active or thread was never terminated.
				m_Lock.Unlock();
			}
		}
		else
			ExitThread(0);
	}
	return (STDLT_OK);
}

// ------------------------------------------------------------------------- //
// NAME: WaitForTerm
//
// per thread / own thread
// ------------------------------------------------------------------------- //
void
CSysThread::WaitForTerm() 
{
	WaitForSingleObject(m_hThread, INFINITE);
} // CSysThread::WaitForTerm

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
unsigned int
CSysThread::ThreadLaunch(
	void * pWhichThread
	) 
{
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
CSysThread::ThreadInit() 
{
	// Anything that needs to be done before control is passed to the
	// thread main loop (ThreadRun)

	return (STDLT_OK);
}

// ------------------------------------------------------------------------- //
// NAME: ThreadTerm
//
// per thread / own thread
// ------------------------------------------------------------------------- //
ESTDLTResults
CSysThread::ThreadTerm() 
{
	// Anything that needs to be done before this thread is completely
	// destroyed

	return (STDLT_OK);
}
