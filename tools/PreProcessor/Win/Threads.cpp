//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//
//------------------------------------------------------------------
//
//	FILE	  : Threads.h
//
//	PURPOSE	  : Implementation for the thread routines.
//
//	CREATED	  : 2nd May 1996
//
//
//------------------------------------------------------------------

#include <windows.h>
//#include "BDefs.h"
#include <process.h>
#include <sys/types.h>
#include <sys/timeb.h>
#include <time.h>
#include "oldtypes.h"
#include "threads.h"

static uint32 g_nProcessors=1, g_CurProcessor=0;


void *thd_CreateCriticalSection()
{
	CRITICAL_SECTION		*pRet;

	pRet = new CRITICAL_SECTION;
	if( pRet )
		InitializeCriticalSection( pRet );

	return pRet;
}

void thd_DeleteCriticalSection( void *pCS )
{
	DeleteCriticalSection( (LPCRITICAL_SECTION)pCS );
	delete pCS;
}


BOOL thd_EnterCriticalSection( void *pCS )
{
	if(!pCS)
		return FALSE;
	
	EnterCriticalSection( (LPCRITICAL_SECTION)pCS );
	return TRUE;
}


void thd_LeaveCriticalSection( void *pCS )
{
	LeaveCriticalSection( (LPCRITICAL_SECTION)pCS );
}



// A ThreadStarter is passed into InternalThreadFn.
class CThreadStarter
{
	public:
		
		void (*pFn)(void *pData);
		void *pData;
};

static unsigned long WINAPI InternalThreadFn(void *pData)
{
	CThreadStarter	starter = *((CThreadStarter*)pData);
	delete (CThreadStarter*)pData;

	starter.pFn(starter.pData);
	return 0;
}


THREAD_ID thd_BeginThread(void (*pFn)(void *pData), void *pArg)
{
	HANDLE			handle;
	CThreadStarter	*pStarter;
	uint32			threadID;

	pStarter = new CThreadStarter;
	pStarter->pFn = pFn;
	pStarter->pData = pArg;
	handle = CreateThread(NULL, 0, InternalThreadFn, pStarter, 0, (unsigned long *)&threadID);

	if(g_nProcessors > 1)
	{
		g_CurProcessor = (g_CurProcessor+1) % g_nProcessors;
	}

	return (uint32)handle;
}

void thd_EndThread()
{
	ExitThread(0);
}


void thd_WaitForFinish(THREAD_ID id)
{
	WaitForSingleObject((HANDLE)id, INFINITE);
}


void thd_WaitForMultipleToFinish(THREAD_ID *pIDs, uint32 nThreads)
{
	HANDLE	*handles = new HANDLE[nThreads];
	uint32	i;

	for(i=0; i < nThreads; i++)
		handles[i] = (HANDLE)pIDs[i];

	WaitForMultipleObjects(nThreads, handles, TRUE, INFINITE);
	delete [] handles;
}


uint32 thd_Init()
{
	SYSTEM_INFO		info;
	uint32			i, nProcessors;

	GetSystemInfo(&info);
	g_nProcessors = nProcessors = info.dwNumberOfProcessors;
	
	for(i=0; i < nProcessors; i++)
		if(!(info.dwActiveProcessorMask & (1<<i)))
			--g_nProcessors;

	if(g_nProcessors == 0)
		g_nProcessors = 1;

	g_CurProcessor = 0;
	return g_nProcessors;
}


void thd_Term()
{
}


void thd_Sleep(uint32 nMilliseconds)
{
	Sleep(nMilliseconds);
}


void thd_GetTimeString(char *pStr)
{
	_tzset();
	_strtime(pStr);
}


void thd_GetDateString(char *pStr)
{
	_tzset();
	_strdate(pStr);
}




