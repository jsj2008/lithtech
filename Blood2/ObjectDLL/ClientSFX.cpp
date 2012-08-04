// ----------------------------------------------------------------------- //
//
// MODULE  : CClientSFX.cpp
//
// PURPOSE : CClientSFX - Base class for client-side sfx objects - Implementation
//
// CREATED : 10/21/97
//
// ----------------------------------------------------------------------- //

#include "ClientSFX.h"


BEGIN_CLASS(CClientSFX)
END_CLASS_DEFAULT_FLAGS(CClientSFX, B2BaseClass, NULL, NULL, CF_HIDDEN)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientSFX::CClientSFX
//
//	PURPOSE:	Initialize id data member
//
// ----------------------------------------------------------------------- //

CClientSFX::CClientSFX(DBYTE nType) : B2BaseClass(nType)
{
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientSFX::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

DDWORD CClientSFX::EngineMessageFn(DDWORD messageID, void *pData, DFLOAT lData)
{
	switch(messageID)
	{
		case MID_PRECREATE:
		{
			ObjectCreateStruct* pStruct = (ObjectCreateStruct*)pData;
			pStruct->m_Flags |= FLAG_FORCECLIENTUPDATE;
			break;
		}
	}
	return B2BaseClass::EngineMessageFn(messageID, pData, lData);
}
