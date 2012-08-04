// *********************************************************************** //
//
// MODULE  : iltthread.h
//
// PURPOSE : Interface class for threads.
//
// CREATED : 05/26/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
//
// *********************************************************************** //

#ifndef __ILTTHREAD_H__
#define __ILTTHREAD_H__

#include "ltbasedefs.h"
//#include "platform.h"

class ILTThread
{
public:

	// thread function type
	typedef uint32 (*UserThreadFunction)(void*);

	// thread argument structure
	struct InternalThreadArgument
	{
		UserThreadFunction m_pUserThreadFunction;
		void*			   m_pArgument;
	};
	
	// create and start the thread
	virtual void Create(UserThreadFunction pUserThreadFunction, void* pArgument) = 0;

	// wait for the thread to exit - note that this is typically used in 
	// conjunction with a thread event to tell the thread to shutdown
	virtual void WaitForExit() = 0;

	// check to see if the thread has been created
	virtual bool IsCreated() = 0;

};


#endif // __ILTTHREAD_H__
