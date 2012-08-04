// ExitHint.cpp: implementation of the CExitHint class.
//
//////////////////////////////////////////////////////////////////////

#include "cpp_server_de.h"
#include "ExitHint.h"

BEGIN_CLASS(ExitHint)
END_CLASS_DEFAULT_FLAGS(ExitHint, B2BaseClass, NULL, NULL, CF_ALWAYSLOAD)

DLink ExitHint::m_ExitHead;
DDWORD ExitHint::m_dwNumExits = 0;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

DDWORD ExitHint::EngineMessageFn(DDWORD messageID, void *pData, float fData)
{
	switch(messageID)
	{
		case MID_INITIALUPDATE:
		{
			// insert it into the list
			dl_Insert( &m_ExitHead, &m_Link );
			m_Link.m_pData = ( void * )this;
			m_dwNumExits++;

			GetServerDE()->SetNextUpdate(m_hObject, 0.0f);
			break;
		}
	}

	return B2BaseClass::EngineMessageFn(messageID, pData, fData);
}
