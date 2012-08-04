
#include "bdefs.h"



Nexus::Nexus()
{
	Init(LTNULL);
}


Nexus::~Nexus()
{
	Term();
}


LTBOOL Nexus::Init(void *pData)
{
	m_LeechHead = LTNULL;
	m_pData = pData;
	return LTTRUE;
}


void Nexus::Term()
{
	LTBOOL bFree;

	bFree = LTTRUE;
	SendMessage(NEXUS_NEXUSDESTROY, &bFree);
}


LTRESULT Nexus::SendMessage(int msg, void *pUserData)
{
	Leech *pCur, *pNext;
	LeechDef *pCurDef;

	for(pCur=m_LeechHead; pCur; pCur=pNext)
	{
		pNext = pCur->m_pNext;

		pCurDef = pCur->m_Def;
		while(pCurDef)
		{
			pCurDef->m_Fn(this, pCur, msg, pUserData);
			pCurDef = pCurDef->m_pParent;
		}
	}

	return LT_OK;
}


void Nexus::RemoveLeech(Leech *pLeech)
{
	Leech **ppPrev, *pCur;

	ppPrev = &m_LeechHead;
	for(pCur=m_LeechHead; pCur; pCur=pCur->m_pNext)
	{
		if(pCur == pLeech)
		{
			*ppPrev = pCur->m_pNext;
			break;
		}

		ppPrev = &pCur->m_pNext;
	}
}


Leech* Nexus::FindLeech(LeechDef *pDef)
{
	Leech *pLeech;

	for(pLeech=m_LeechHead; pLeech; pLeech=pLeech->m_pNext)
	{
		if(pLeech->m_Def == pDef)
			return pLeech;
	}
	
	return LTNULL;
}
