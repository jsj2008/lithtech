// ----------------------------------------------------------------------- //
//
// MODULE  : BulletImpactSFX.cpp
//
// PURPOSE : BulletImpactSFX - Implementation
//
// CREATED : 10/23/97
//
// ----------------------------------------------------------------------- //

#include "BulletImpactSFX.h"
#include "cpp_server_de.h"
#include "SFXMsgIds.h"


BEGIN_CLASS(CBulletImpactSFX)
END_CLASS_DEFAULT_FLAGS(CBulletImpactSFX, CClientSFX, NULL, NULL, CF_HIDDEN)



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBulletImpactSFX::Setup
//
//	PURPOSE:	Send message to client with data
//
// ----------------------------------------------------------------------- //

void CBulletImpactSFX::Setup( DVector *pvPos, DVector *pvDir, DFLOAT fScale, HSTRING hstrSprite )
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	DRotation rotation;
	pServerDE->AlignRotation( &rotation, pvDir, pvDir );

	// Tell the clients about the Mark, and remove thyself...

	HMESSAGEWRITE hMessage = pServerDE->StartSpecialEffectMessage(this);
	pServerDE->WriteToMessageByte(hMessage, SFX_MARK_ID);
//pServerDE->WriteToMessageByte(hMessage, DTRUE);
	pServerDE->WriteToMessageRotation(hMessage, &rotation);
	pServerDE->WriteToMessageFloat( hMessage, fScale );
	pServerDE->WriteToMessageHString( hMessage, hstrSprite );
	pServerDE->EndMessage(hMessage);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBulletImpactSFX::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

DDWORD CBulletImpactSFX::EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData)
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

	return CClientSFX::EngineMessageFn(messageID, pData, fData);
}
