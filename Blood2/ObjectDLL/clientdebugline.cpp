
// ----------------------------------------------------------------------- //
//
// MODULE  : ClientDebugLine.cpp
//
// PURPOSE : ClientDebugLine - Implementation
//
// CREATED : 10/23/97
//
// ----------------------------------------------------------------------- //

#include "ClientDebugLine.h"
#include "cpp_server_de.h"
#include "SFXMsgIds.h"

BEGIN_CLASS(CClientDebugLine)
END_CLASS_DEFAULT_FLAGS(CClientDebugLine, CClientSFX, NULL, NULL, CF_HIDDEN)



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientDebugLine::Setup
//
//	PURPOSE:	Setup the tracer
//
// ----------------------------------------------------------------------- //

void CClientDebugLine::Setup(DVector *pvFrom, DVector *pvTo)
{ 
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	// Tell the clients about the tracer, and remove thyself...

	DVector vOrigin, vPoint1, vPoint2;
	pServerDE->GetObjectPos(pServerDE->ObjectToHandle(this), &vOrigin);

	VEC_SUB(vPoint1, vOrigin, *pvFrom);
	VEC_SUB(vPoint2, vOrigin, *pvTo);

	HMESSAGEWRITE hMessage = pServerDE->StartSpecialEffectMessage(this);
	pServerDE->WriteToMessageByte(hMessage, SFX_DEBUGLINE_ID);
	pServerDE->WriteToMessageVector(hMessage, &vPoint1);
	pServerDE->WriteToMessageVector(hMessage, &vPoint2);
	pServerDE->EndMessage(hMessage);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientDebugLine::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

DDWORD CClientDebugLine::EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData)
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
