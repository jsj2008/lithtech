// *********************************************************************** //
//
// MODULE  : linux_ltthreadevent.h
//
// PURPOSE : Linux implementation class for thread events.
//
// CREATED : 05/25/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
//
// *********************************************************************** //

#ifndef __LINUX_LTTHREADEVENT_H__
#define __LINUX_LTTHREADEVENT_H__

#include "iltthreadevent.h"
#include <sys/time.h>
#include <pthread.h>
#include <asm/errno.h>

enum Linux_LTThreadEventType
{
	AutoReset,
	ManualReset,
};

template<Linux_LTThreadEventType eThreadEventType>
class CLinux_LTThreadEvent : protected ILTThreadEvent
{
public:

	CLinux_LTThreadEvent();
	~CLinux_LTThreadEvent();

	// set the event to the signaled state
	virtual void Set();

	// set the event to the non-signaled state
	virtual void Clear();

	// wait for the event to be signaled
	virtual LTThreadEventBlockResult Block(uint32 nTimeout = INFINITE);

	// check to see if the event is set
	virtual bool IsSet();

private:

	// flag indicating the state of the event
	bool m_bIsSignaled;

	// number of waiting threads
	uint32 m_nNumberOfWaitingThreads;
	
	// handle to the Linux mutex and condition objects
	pthread_mutex_t m_hMutex;
	pthread_cond_t  m_hCondition;	
};

template<Linux_LTThreadEventType eThreadEventType>
CLinux_LTThreadEvent<eThreadEventType>::CLinux_LTThreadEvent()
	: m_bIsSignaled(false),
	  m_nNumberOfWaitingThreads(0)
{
	::pthread_mutex_init(&m_hMutex, NULL);
	::pthread_cond_init(&m_hCondition, NULL);
}	

template<Linux_LTThreadEventType eThreadEventType>
CLinux_LTThreadEvent<eThreadEventType>::~CLinux_LTThreadEvent()
{
	::pthread_mutex_destroy(&m_hMutex);
	::pthread_cond_destroy(&m_hCondition);
}

template<Linux_LTThreadEventType eThreadEventType>
void CLinux_LTThreadEvent<eThreadEventType>::Set()
{
	::pthread_mutex_lock(&m_hMutex);
	
	if (eThreadEventType == ManualReset)
	{
		// set state to signaled
		m_bIsSignaled = true;
		
		// release all waiting threads
		::pthread_cond_broadcast(&m_hCondition);
	}
	else if (eThreadEventType == AutoReset)
	{
		if (m_nNumberOfWaitingThreads == 0)
		{
			// no waiting threads - set signaled state
			m_bIsSignaled = true;
		}
		else
		{
			// release one waiting thread
			::pthread_cond_signal(&m_hCondition);
		}
	}
	
	::pthread_mutex_unlock(&m_hMutex);
}

template<Linux_LTThreadEventType eThreadEventType>
void CLinux_LTThreadEvent<eThreadEventType>::Clear()
{
	::pthread_mutex_lock(&m_hMutex);
	m_bIsSignaled = false;
	::pthread_mutex_unlock(&m_hMutex);
}

template<Linux_LTThreadEventType eThreadEventType>
LTThreadEventBlockResult CLinux_LTThreadEvent<eThreadEventType>::Block(uint32 nTimeout /* = INFINITE */)
{
	::pthread_mutex_lock(&m_hMutex);

	// check to see if we can avoid blocking based on the current state
	if (eThreadEventType == ManualReset)
	{
		if (m_bIsSignaled == true)
		{
			// already signaled - unlock the mutext and return signaled
			::pthread_mutex_unlock(&m_hMutex);
			
			return BLOCK_EVENTSET;
		}
	}
	else if (eThreadEventType == AutoReset)
	{
		if (m_bIsSignaled == true)
		{
			// unlock and clear the signaled state since we only release one thread in auto-reset mode
			::pthread_mutex_unlock(&m_hMutex);
			
			m_bIsSignaled = false;
			
			return BLOCK_EVENTSET;
		}
	}
	
	// need to block on the condition - calculate blocking time if it's non-infinite
	++m_nNumberOfWaitingThreads;
	int waitResult;
	
	if (nTimeout != INFINITE)
	{
		timeval timeNow;
		::gettimeofday(&timeNow, NULL);
		
		timespec timeToWait;
		timeToWait.tv_sec = timeNow.tv_sec;
		timeToWait.tv_nsec = (timeNow.tv_usec * 1000) + (nTimeout * 1000000);

		waitResult = ::pthread_cond_timedwait(&m_hCondition, &m_hMutex, &timeToWait);
	}
	else
	{
		waitResult = ::pthread_cond_wait(&m_hCondition, &m_hMutex);
	}

	--m_nNumberOfWaitingThreads;
	
	// we need to unlock the mutex since pthread_cond_timewait and pthread_cond_wait
	// will re-acquire it after the condition is set
	::pthread_mutex_unlock(&m_hMutex);
	
	if (waitResult == ETIMEDOUT)
	{
		return BLOCK_TIMEOUT;
	}
	else if (waitResult == 0)
	{
		return BLOCK_EVENTSET;
	}
	else
	{
		return (LTThreadEventBlockResult)waitResult;
	}
}

template<Linux_LTThreadEventType eThreadEventType>
bool CLinux_LTThreadEvent<eThreadEventType>::IsSet()
{
	bool bResult;
	
	::pthread_mutex_lock(&m_hMutex);
	bResult = m_bIsSignaled;
	::pthread_mutex_unlock(&m_hMutex);
	
	return bResult;
}

#endif // __LINUX_LTTHREADEVENT_H__
