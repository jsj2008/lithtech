// ----------------------------------------------------------------------- //
//
// MODULE  : CClientSplatFX.cpp
//
// PURPOSE : CClientSplatFX - Implementation
//
// CREATED : 11/6/97
//
// ----------------------------------------------------------------------- //

#include "ClientSplatFX.h"
#include "cpp_server_de.h"
#include "SFXMsgIds.h"


BEGIN_CLASS(CClientSplatFX)
END_CLASS_DEFAULT_FLAGS(CClientSplatFX, CClientSFX, NULL, NULL, CF_HIDDEN)


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientSplatFX::Setup
//
//	PURPOSE:	Send message to client with data
//
// ----------------------------------------------------------------------- //

void CClientSplatFX::Setup( DVector *pvPos, DVector *pvDir, DFLOAT fScale, DFLOAT fGrowScale)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	DRotation rotation;
	pServerDE->AlignRotation( &rotation, pvDir, pvDir );

	HMESSAGEWRITE hMessage = pServerDE->StartInstantSpecialEffectMessage(pvPos);
	pServerDE->WriteToMessageByte(hMessage, SFX_BLOODSPLAT_ID);
	pServerDE->WriteToMessageCompPosition(hMessage, pvPos);
	pServerDE->WriteToMessageRotation(hMessage, &rotation);
	pServerDE->WriteToMessageFloat( hMessage, fScale );
	pServerDE->WriteToMessageFloat( hMessage, fGrowScale );
	pServerDE->EndMessage(hMessage);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientSplatFX::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

DDWORD CClientSplatFX::EngineMessageFn(DDWORD messageID, void *pData, DFLOAT lData)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return 0;

	switch(messageID)
	{
		case MID_INITIALUPDATE:
		{
			pServerDE->SetNextUpdate(m_hObject, 0.0001f);
			break;
		}

		case MID_UPDATE:
		{
			pServerDE->RemoveObject(m_hObject);
			break;
		}

		default : break;
	}

	return CClientSFX::EngineMessageFn(messageID, pData, lData);
}
