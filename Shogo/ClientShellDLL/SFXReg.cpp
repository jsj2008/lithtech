
#include "iltclient.h"
#include "SFXReg.h"


SFXReg *g_SFXRegHead = NULL;


SFXReg* FindSFXReg(uint32 id)
{
	SFXReg *pCur;

	for(pCur=g_SFXRegHead; pCur; pCur=pCur->m_pNext)
	{
		if(pCur->m_ID == id)
			return pCur;
	}

	return NULL;
}


SFXReg* FindSFXRegByName(char *pName)
{
	SFXReg *pCur;

	for(pCur=g_SFXRegHead; pCur; pCur=pCur->m_pNext)
	{
		if(strcmp(pCur->m_SFXName, pName) == 0)
			return pCur;
	}

	return NULL;
}

