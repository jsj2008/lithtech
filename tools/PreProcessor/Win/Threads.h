//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//
//------------------------------------------------------------------
//
//	FILE	  : Threads.h
//
//	PURPOSE	  : Puts wrappers on the thread functions for a 
//              particular OS.
//
//	CREATED	  : 2nd May 1996
//
//
//------------------------------------------------------------------

#ifndef __THREADS_H__
	#define __THREADS_H__


	// Includes....
//	#include "BDefs.h"
#include "ltbasetypes.h"

	// Defines....
	#define THREAD_ID			uint32
	#define INVALID_THREAD_ID	0



	// Functions....

	// Initializes and returns the number of processors on this system.
	uint32			thd_Init();
	void			thd_Term();

	void			thd_Sleep(uint32 nMilliseconds);
	
	void*			thd_CreateCriticalSection();
	void			thd_DeleteCriticalSection( void *pCS );

	// Returns FALSE if there is an error.
	BOOL			thd_EnterCriticalSection( void *pCS );
	void			thd_LeaveCriticalSection( void *pCS );
	
	
	// Call this to start a new thread.
	// Returns INVALID_THREAD_ID if there is an error.
	THREAD_ID		thd_BeginThread( void (*pFn)(void *pData), void *pArg );
	
	// Call this from inside the thread function before it exits.
	void			thd_EndThread();

	// Wait for a thread to finish.
	void			thd_WaitForFinish(THREAD_ID id);
	void			thd_WaitForMultipleToFinish(THREAD_ID *pIDs, uint32 nThreads);

	// Ok, these aren't related to threads....
	void			thd_GetTimeString(char *pStr);
	void			thd_GetDateString(char *pStr);


#endif




