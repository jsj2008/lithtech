// ----------------------------------------------------------------------- //
//
// MODULE  : ClientTracer.cpp
//
// PURPOSE : ClientTracer - Implementation
//
// CREATED : 10/23/97
//
// ----------------------------------------------------------------------- //

#include "ClientTracer.h"
#include "cpp_server_de.h"
#include "SFXMsgIds.h"


BEGIN_CLASS(CClientTracer)
END_CLASS_DEFAULT_FLAGS(CClientTracer, CClientSFX, NULL, NULL, CF_HIDDEN)



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientTracer::Setup
//
//	PURPOSE:	Setup the tracer
//
// ----------------------------------------------------------------------- //

void CClientTracer::Setup(DVector *pvTo, HOBJECT hGun)
{ 
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	// Tell the clients about the tracer, and remove thyself...

	HMESSAGEWRITE hMessage = pServerDE->StartSpecialEffectMessage(this);
	pServerDE->WriteToMessageByte(hMessage, SFX_TRACER_ID);
//pServerDE->WriteToMessageByte(hMessage, DFALSE);	// Static
	pServerDE->WriteToMessageVector(hMessage, pvTo);
	pServerDE->WriteToMessageObject(hMessage, hGun);
	pServerDE->EndMessage(hMessage);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientTracer::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

DDWORD CClientTracer::EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData)
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
