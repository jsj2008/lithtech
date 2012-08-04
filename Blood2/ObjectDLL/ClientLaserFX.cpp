// ----------------------------------------------------------------------- //
//
// MODULE  : ClientLaserFX.cpp
//
// PURPOSE : ClientLaserFX - Implementation
//
// CREATED : 2/28/98
//
// ----------------------------------------------------------------------- //

#include "ClientLaserFX.h"
#include "cpp_server_de.h"
#include "SFXMsgIds.h"


BEGIN_CLASS(CClientLaserFX)
END_CLASS_DEFAULT_FLAGS(CClientLaserFX, CClientSFX, NULL, NULL, CF_HIDDEN)



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientLaserFX::Setup
//
//	PURPOSE:	Setup the tracer
//
// ----------------------------------------------------------------------- //

void CClientLaserFX::Setup(HOBJECT hGun)
{ 
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	HMESSAGEWRITE hMessage = pServerDE->StartSpecialEffectMessage(this);
	pServerDE->WriteToMessageByte(hMessage, SFX_LASER_ID);
//pServerDE->WriteToMessageByte(hMessage, DFALSE);	
	pServerDE->WriteToMessageObject(hMessage, hGun);
	pServerDE->EndMessage(hMessage);
}

