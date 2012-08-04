// *********************************************************************** //
//
// MODULE  : win32_ltthreadevent.h
//
// PURPOSE : Win32 implementation class for thread events.
//
// CREATED : 05/25/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
//
// *********************************************************************** //

#ifndef __WIN32_LTTHREADEVENT_H__
#define __WIN32_LTTHREADEVENT_H__

#include "iltthreadevent.h"

enum Win32_LTThreadEventType
{
	AutoReset,
	ManualReset
};

template<Win32_LTThreadEventType eThreadEventType>
class CWin32_LTThreadEvent : protected ILTThreadEvent
{
public:

	CWin32_LTThreadEvent();
	~CWin32_LTThreadEvent();

	// set the event to the signaled state
	virtual void Set();

	// set the event to the non-signaled state
	virtual void Clear();

	// wait for the event to be signaled
	virtual LTThreadEventBlockResult Block(uint32 nTimeoutMS = INFINITE);

	// check to see if the event is set
	virtual bool IsSet();

	// Win32-only: get the event handle
	HANDLE GetEvent();

private:

	// handle to the Win32 event object
	HANDLE m_hEvent;
};

template<Win32_LTThreadEventType eThreadEventType>
CWin32_LTThreadEvent<eThreadEventType>::CWin32_LTThreadEvent()
	:  m_hEvent(INVALID_HANDLE_VALUE)
{
	BOOL bManualReset = FALSE;

	if (eThreadEventType == ManualReset)
	{
		bManualReset = TRUE;
	}
	
	m_hEvent = ::CreateEvent(NULL, bManualReset, FALSE, NULL);
}	

template<Win32_LTThreadEventType eThreadEventType>
CWin32_LTThreadEvent<eThreadEventType>::~CWin32_LTThreadEvent()
{
	if (m_hEvent != INVALID_HANDLE_VALUE)
	{
		::CloseHandle(m_hEvent);
	}

	m_hEvent = INVALID_HANDLE_VALUE;
}

template<Win32_LTThreadEventType eThreadEventType>
inline void CWin32_LTThreadEvent<eThreadEventType>::Set()
{
	ASSERT(m_hEvent != INVALID_HANDLE_VALUE); //, "Attempt to SetEvent with invalid handle");

	::SetEvent(m_hEvent);
}

template<Win32_LTThreadEventType eThreadEventType>
inline void CWin32_LTThreadEvent<eThreadEventType>::Clear()
{
	ASSERT(m_hEvent != INVALID_HANDLE_VALUE ); //, "Attempt to ResetEvent with invalid handle");

	::ResetEvent(m_hEvent);
}

template<Win32_LTThreadEventType eThreadEventType>
inline LTThreadEventBlockResult CWin32_LTThreadEvent<eThreadEventType>::Block(uint32 nTimeout /* = INFINITE */)
{
	ASSERT(m_hEvent != INVALID_HANDLE_VALUE ); //, "Attempt to WaitForSingleObject with invalid handle");

	DWORD nWaitResult = ::WaitForSingleObject(m_hEvent, nTimeout);
	if (nWaitResult == WAIT_TIMEOUT)
	{
		return BLOCK_TIMEOUT;
	}
	else if (nWaitResult == WAIT_OBJECT_0)
	{
		return BLOCK_EVENTSET;
	}
	else
	{
		return (LTThreadEventBlockResult) ::GetLastError();
	}
}

template<Win32_LTThreadEventType eThreadEventType>
inline bool CWin32_LTThreadEvent<eThreadEventType>::IsSet()
{
	ASSERT(m_hEvent != INVALID_HANDLE_VALUE ); //, "Attempt to WaitForSingleObject with invalid handle");

	return WaitForSingleObject(m_hEvent, 0) == WAIT_OBJECT_0;
}

template<Win32_LTThreadEventType eThreadEventType>
inline HANDLE CWin32_LTThreadEvent<eThreadEventType>::GetEvent()
{
	return m_hEvent;
}

#endif // __WIN32_LTTHREADEVENT_H__
