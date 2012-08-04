
// SmellHint.cpp: implementation of the CExitHint class.
//
//////////////////////////////////////////////////////////////////////

#include "cpp_server_de.h"
#include "SmellHint.h"
#include "clientdebugline.h"

BEGIN_CLASS(SmellHint)
END_CLASS_DEFAULT_FLAGS(SmellHint, B2BaseClass, NULL, NULL, CF_ALWAYSLOAD)

DLink SmellHint::m_SmellHead;
DDWORD SmellHint::m_dwNumSmells = 0;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

DDWORD SmellHint::EngineMessageFn(DDWORD messageID, void *pData, float fData)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return 0;

	switch(messageID)
	{
		case MID_INITIALUPDATE:
		{
			// insert it into the list
			dl_Insert( m_SmellHead.m_pPrev, &m_Link );
			m_Link.m_pData = ( void * )this;
			m_dwNumSmells++;

			pServerDE->SetNextUpdate(m_hObject, MAX_SMELL_LIFE);
			break;
		}

		case MID_UPDATE:
		{
			pServerDE->RemoveObject(m_hObject);
			break;
		}
	}

	return B2BaseClass::EngineMessageFn(messageID, pData, fData);
}


DLink* SmellHint::HandleToLink(HOBJECT hObj)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return DNULL;

	DLink* pLink = m_SmellHead.m_pNext;

	if(pLink == DNULL)
		return DNULL;

	while(pLink != &m_SmellHead)
	{
		SmellHint* pHint = (SmellHint*)pLink->m_pData;
		HOBJECT hHintObj = pServerDE->ObjectToHandle(pHint);

		if(hHintObj == hObj)
			return pLink;

		pLink = pLink->m_pNext;

		if(pLink == DNULL)
			return DNULL;
	}

	return DNULL;
}