// ----------------------------------------------------------------------- //
//
// MODULE  : CClientSmokeTrail.cpp
//
// PURPOSE : CClientSmokeTrail - Implementation
//
// CREATED : 10/21/97
//
// ----------------------------------------------------------------------- //

#include "ClientSmokeTrail.h"
#include "cpp_server_de.h"
#include "SFXMsgIds.h"


BEGIN_CLASS(CClientSmokeTrail)
END_CLASS_DEFAULT_FLAGS(CClientSmokeTrail, CClientSFX, NULL, NULL, CF_HIDDEN)


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientSmokeTrail::Setup
//
//	PURPOSE:	Setup the smoke trail
//
// ----------------------------------------------------------------------- //

void CClientSmokeTrail::Setup(DVector vVel, DBOOL bSmall)
{ 
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	// Tell the clients about the SmokeTrail, and remove thyself...

	HMESSAGEWRITE hMessage = pServerDE->StartSpecialEffectMessage(this);
	pServerDE->WriteToMessageByte(hMessage, SFX_SMOKETRAIL_ID);
//pServerDE->WriteToMessageByte(hMessage, DFALSE);
	pServerDE->WriteToMessageVector(hMessage, &vVel);
	pServerDE->WriteToMessageByte(hMessage, bSmall);
	pServerDE->EndMessage(hMessage);
}