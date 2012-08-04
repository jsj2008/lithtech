// *********************************************************************** //
//
// MODULE  : win32_ltthread.h
//
// PURPOSE : Win32 implementation thread wrapper class.
//
// CREATED : 05/26/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
//
// *********************************************************************** //

#ifndef __WIN32_LTTHREAD_H__
#define __WIN32_LTTHREAD_H__

#include "iltthread.h"

class CWin32_LTThread : protected ILTThread
{
public:

	CWin32_LTThread();
	virtual ~CWin32_LTThread();

	// create and start the thread
	virtual void Create(UserThreadFunction pUserThreadFunction, void* pArgument);

	// wait for the thread to exit - note that this is typically used in 
	// conjunction with a thread event to tell the thread to shutdown
	virtual void WaitForExit();

	// check to see if the thread has been created
	virtual bool IsCreated();

private:

	HANDLE m_hThread;

	// platform-specific internal thread function
	static DWORD WINAPI InternalThreadFunction(LPVOID pArgument);

	// argument used with platform-specific thread function
	InternalThreadArgument m_threadArgument;
};

inline CWin32_LTThread::CWin32_LTThread()
	: m_hThread(INVALID_HANDLE_VALUE)
{
}

inline CWin32_LTThread::~CWin32_LTThread()
{
}

inline void CWin32_LTThread::Create(UserThreadFunction pUserThreadFunction, void* pArgument)
{
	m_threadArgument.m_pUserThreadFunction = pUserThreadFunction;
	m_threadArgument.m_pArgument = pArgument;

	DWORD nThreadID;
	m_hThread = ::CreateThread(NULL, 0, InternalThreadFunction, &m_threadArgument, 0, &nThreadID);

	ASSERT(m_hThread != INVALID_HANDLE_VALUE); //, "Failed to create thread");
}

inline DWORD WINAPI CWin32_LTThread::InternalThreadFunction(LPVOID pArgument)
{
	// get the argument structure
	InternalThreadArgument* internalThreadArgument = (InternalThreadArgument*) pArgument;

	// get the user's thread function and argument
	UserThreadFunction pUserThreadFunction = internalThreadArgument->m_pUserThreadFunction;
	void*              pUserThreadArgument = internalThreadArgument->m_pArgument;

	// call the user thread function
	uint32 returnValue = pUserThreadFunction(pUserThreadArgument);

	return (DWORD) returnValue;
}

inline void CWin32_LTThread::WaitForExit()
{
	ASSERT(m_hThread != INVALID_HANDLE_VALUE ); //, "Attempt to WaitForSingleObject with invalid handle");

	// temporary: 1-second timeout or we terminate the thread
	if (::WaitForSingleObject(m_hThread, 1000) == WAIT_TIMEOUT)
	{
		//::TerminateThread(m_hThread, NULL);
	}

	::CloseHandle(m_hThread);
	m_hThread = INVALID_HANDLE_VALUE;
}

inline bool CWin32_LTThread::IsCreated()
{
	return m_hThread != INVALID_HANDLE_VALUE ? true : false;
}

#endif // __WIN32_LTTHREAD_H__
