// ----------------------------------------------------------------------- //
//
// MODULE  : CClientCastLineSFX.cpp
//
// PURPOSE : CClientCastLineSFX - Implementation
//
// CREATED : 1/27/98
//
// ----------------------------------------------------------------------- //

#include "ClientCastLineSFX.h"
#include "cpp_server_de.h"
#include "SFXMsgIds.h"


BEGIN_CLASS(CClientCastLineSFX)
END_CLASS_DEFAULT_FLAGS(CClientCastLineSFX, CClientSFX, NULL, NULL, CF_HIDDEN)


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientCastLineSFX::Setup
//
//	PURPOSE:	Setup the line system
//
// ----------------------------------------------------------------------- //

void CClientCastLineSFX::Setup(DVector vStartColor, DVector vEndColor,
							   DFLOAT fStartAlpha, DFLOAT fEndAlpha)
{ 
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	// Tell the clients about the line...

	HMESSAGEWRITE hMessage = pServerDE->StartSpecialEffectMessage(this);
	pServerDE->WriteToMessageByte(hMessage, SFX_CASTLINE_ID);
	pServerDE->WriteToMessageVector(hMessage, &vStartColor);
	pServerDE->WriteToMessageVector(hMessage, &vEndColor);
	pServerDE->WriteToMessageFloat(hMessage, fStartAlpha);
	pServerDE->WriteToMessageFloat(hMessage, fEndAlpha);
	pServerDE->EndMessage(hMessage);
}