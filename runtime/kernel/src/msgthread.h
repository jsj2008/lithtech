// ------------------------------------------------------------------------- //
//
// FILE      : M S G T H R E A D . H
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

#ifndef __MSGTHREAD_H__
#define __MSGTHREAD_H__

// ------------------------------------------------------------------------- //
// Includes this module depends on
// ------------------------------------------------------------------------- //
#include "stdlterror.h"

#include "systhread.h"

// ------------------------------------------------------------------------- //
// Typedefs & Enums
// ------------------------------------------------------------------------- //

// ------------------------------------------------------------------------- //
// Class Definitions
// ------------------------------------------------------------------------- //

class CMsgThread : public CSysThread {

	public:
		// Message to the thread go in here.
		LThreadQueue	m_Incoming;
		// The thread posts status messages in here.
		LThreadQueue	m_Outgoing;

		ESTDLTResults	PostMessage(LThreadMessage &msg) {
			return m_Incoming.PostMessage(msg);
		}
};

#endif  // __MSGTHREAD_H__