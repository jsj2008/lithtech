// ------------------------------------------------------------------------- //
//
// FILE      : I T H R E A D . H
//
// CREATED   : 11/05/99
//
// AUTHOR    :
//
// COPYRIGHT : Monolith Productions Inc.
//
// ORIGN     : lithtech 1.5/2.0
//
// ------------------------------------------------------------------------- //

#ifndef __ITHREAD_H__
#define __ITHREAD_H__

#ifndef __STDLTERROR_H__
#include "stdlterror.h"
#endif

#ifndef __LTINTEGER_H__
#include "ltinteger.h"
#endif

// ------------------------------------------------------------------------- //
// Typedefs & Enums
// ------------------------------------------------------------------------- //
enum EThreadPriority {
	ThreadLow    =0,
	ThreadPri0   =0,
	ThreadPri1   =1,
	ThreadNormal =2,
	ThreadPri2   =2,
	ThreadPri3   =3,
	ThreadHigh   =4,
	ThreadPri4   =4,
};

enum ESyncResult {
	SYNC_OK      = 0,
	SYNC_ERROR   = 1,
	SYNC_TIMEOUT = 2
};

// ------------------------------------------------------------------------- //
// Class Definitions
// ------------------------------------------------------------------------- //

class IThread {

	public:
		virtual ESTDLTResults Run    (EThreadPriority pri = ThreadNormal) = 0;
		virtual ESTDLTResults Wakeup () = 0;
		virtual ESTDLTResults Term   (bool blocking = true) = 0;
        virtual void          WaitForTerm() = 0;
	protected:
		virtual void		  Sleep  () = 0;
};

class ISyncVar {
	public:
		virtual ESyncResult Begin() = 0;
		virtual ESyncResult End() = 0;
		virtual ESyncResult Wait() = 0;
};

class ISerialVar {
	public:
		virtual ESyncResult Lock() = 0;
		virtual ESyncResult Unlock() = 0;
};

class IEventVar : ISerialVar {
// WaitFor unlocks mutex and then waits up to timeout_ms for the event to be 
// signalled.  If the event is signalled during that period, the mutex is 
// reacquired.
//
// This is the standard method of dealing with pthread condition objects, and
// in the interest of having events/conditions work the same way on all platforms
// we're using it everywhere.
//
// If you want something more similar to a Win32 Event, you'll need find some
// way to support it with pthreads.  I didn't see an easy way to do that...
	public:
		virtual ESyncResult Signal() = 0;
		virtual ESyncResult WaitFor(uint32 timeout_ms) =0;
};

#endif  // __ITHREAD_H__

