// *********************************************************************** //
//
// MODULE  : iltthreadevent.h
//
// PURPOSE : Interface class for thread events.
//
// CREATED : 05/25/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
//
// *********************************************************************** //

#ifndef __ILTTHREADEVENT_H__
#define __ILTTHREADEVENT_H__

#include "ltbasedefs.h"
//#include "platform.h"

#ifdef __LINUX
#define INFINITE (uint32)-1
#endif

enum LTThreadEventBlockResult
{
	BLOCK_EVENTSET,
	BLOCK_TIMEOUT
};

class ILTThreadEvent
{
public:

	// set the event to the signaled state
	virtual void Set() = 0;
	
	// set the event to the non-signaled state
	virtual void Clear() = 0;

	// wait for the event to be signaled
	virtual LTThreadEventBlockResult Block(uint32 nTimeoutMS = INFINITE) = 0;

	// check to see if the event is set
	virtual bool IsSet() = 0;

};

#endif // __LTTHREADEVENT_H__
