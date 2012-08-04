// *********************************************************************** //
//
// MODULE  : linux_ltthread.cpp
//
// PURPOSE : Linux implementation of thread wrapper class.
//
// CREATED : 05/26/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
//
// *********************************************************************** //

#include "linux_ltthread.h"

extern "C" 
{
	// platform-specific internal thread function
	static void* InternalThreadFunction(void* pArgument);
}


void CLinux_LTThread::Create(UserThreadFunction pUserThreadFunction, void* pArgument)
{
	ASSERT(m_hThread == 0 ); //, "Attempt to create thread that has already been created");
	
	m_threadArgument.m_pUserThreadFunction = pUserThreadFunction;
	m_threadArgument.m_pArgument = pArgument;

	int returnValue = ::pthread_create(&m_hThread, NULL, &InternalThreadFunction, (void*)&m_threadArgument);
	
	ASSERT(returnValue == 0 ); //, "Failed to create thread");
}

void CLinux_LTThread::WaitForExit()
{
	ASSERT(m_hThread != 0 ); //, "Attempt to WaitForExit with invalid handle");

	::pthread_join(m_hThread, NULL);
	
	m_hThread = 0;
}

void* InternalThreadFunction(void* pArgument)
{
	// get the argument structure
	ILTThread::InternalThreadArgument* internalThreadArgument = (ILTThread::InternalThreadArgument*) pArgument;

	// get the user's thread function and argument
	ILTThread::UserThreadFunction pUserThreadFunction = internalThreadArgument->m_pUserThreadFunction;
	void*      			          pUserThreadArgument = internalThreadArgument->m_pArgument;

	// call the user thread function
	uint32 returnValue = pUserThreadFunction(pUserThreadArgument);

	return (void*) returnValue;
}


