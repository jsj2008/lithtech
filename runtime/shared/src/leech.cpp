
#include "bdefs.h"
#include "nexus.h"
#include "syslthread.h"



ObjectBank<Leech, LCriticalSection> g_LeechBank(32, 32);
LCriticalSection g_NexusCS;



// The base Leech stuff.
LTRESULT BaseLeechFn(Nexus *pNexus, Leech *pLeech, int msg, void *pUserData)
{
	LTBOOL bFree;

	if(msg == NEXUS_NEXUSDESTROY)
	{
		// Remove it from the Nexus' list.
		pNexus->RemoveLeech(pLeech);

		if(pUserData)
		{
			bFree = *((LTBOOL*)pUserData);
			if(bFree)
			{
				g_LeechBank.Free(pLeech);
			}
		}
	}

	return LT_OK;
}


LeechDef g_BaseLeech =
{
	BaseLeechFn,
	LTNULL
};		


Leech* nexus_CreateLeech(LeechDef *pDef, void *pUserData)
{
	Leech *pRet;
	
	pRet = g_LeechBank.Allocate();
	if(pRet)
	{
		pRet->m_Def = pDef;
		pRet->m_pUserData = pUserData;
	}
	
	return pRet;
} 


LTRESULT nexus_AddLeech(Nexus *pNexus, Leech *pLeech)
{
	if(pLeech)
	{
		CSAccess cs(&g_NexusCS);

		pLeech->m_pNext = pNexus->m_LeechHead;
		pNexus->m_LeechHead = pLeech;
		return LT_OK;
	}
	else
	{
		return LT_ERROR;
	}
}

