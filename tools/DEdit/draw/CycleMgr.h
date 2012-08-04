//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//

// Cyclemgr just provides useful routines for counting processor
// cycles in sections of code.

#ifndef __CYCLEMGR_H__
#define __CYCLEMGR_H__


	typedef struct
	{
		uint32	m_CyclesHigh;
		uint32	m_CyclesLow;
		LARGE_INTEGER	m_Count;
	} CycleMgr;


	// Call this before running the section of code you want to run.
	__inline void StartCycleCount(CycleMgr *pMgr)
	{
		//pMgr->m_Count = timeGetTime();

/*
		__asm
		{
			_emit	0x0F
			_emit	0x31
			mov		[pMgr], edx
			add		pMgr, 4
			mov		[pMgr], eax
		}
*/
		QueryPerformanceCounter(&pMgr->m_Count);
	}


	// Returns number of milliseconds since the StartCycleCount call.
	__inline uint32 EndCycleCount(CycleMgr *pMgr)
	{
		//return timeGetTime() - pMgr->m_Count;

/*
		uint32 retVal;

		__asm
		{
			_emit	0x0F
			_emit	0x31
			mov		ecx, pMgr
			add		ecx, 4
			sub		eax, [ecx]
			sbb		edx, [pMgr]
			mov		retVal, eax
		}

		return retVal >> 8;
*/
		LARGE_INTEGER curVal;
		QueryPerformanceCounter(&curVal);
		return curVal.LowPart - pMgr->m_Count.LowPart;
	}


	// You can use one of these objects to count a function's execution length
	// easily.  Pass in the address of a DWORD to be incremented with the object's lifetime.
	class CCycleCounter
	{
		public:

					CCycleCounter(uint32 *pAddTo)
					{
						m_pAddTo = pAddTo;
						StartCycleCount(&m_Mgr);
					}

					~CCycleCounter()
					{
						EndCount();
					}

			void	EndCount()
			{
				if(m_pAddTo)
				{
					*m_pAddTo += EndCycleCount(&m_Mgr);
					m_pAddTo = 0;
				}
			}

			CycleMgr m_Mgr;
			DWORD *m_pAddTo;

	};


#endif







