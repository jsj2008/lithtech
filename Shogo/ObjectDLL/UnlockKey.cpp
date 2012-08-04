// ----------------------------------------------------------------------- //
//
// MODULE  : UnlockKey.cpp
//
// PURPOSE : Riot key for unlocking objects (like doors) - Implementation
//
// CREATED : 10/27/97
//
// ----------------------------------------------------------------------- //

#include "UnlockKey.h"
#include "RiotMsgIds.h"
#include "cpp_server_de.h"

// ----------------------------------------------------------------------- //
// ----------------------------------------------------------------------- //
//
//	CLASS:		UnlockKey
//
//	PURPOSE:	Keys for unlocking game objects
//
// ----------------------------------------------------------------------- //
// ----------------------------------------------------------------------- //

BEGIN_CLASS(UnlockKey)
	ADD_STRINGPROP(Skin, "")
END_CLASS_DEFAULT(UnlockKey, Powerup, NULL, NULL)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	UnlockKey::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

DDWORD UnlockKey::EngineMessageFn(DDWORD messageID, void *pData, DFLOAT lData)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return DFALSE;

	switch(messageID)
	{
		case MID_INITIALUPDATE:
		{
			DVector vDims;
			VEC_SET(vDims, 20.0f, 25.0f, 10.0f);

			pServerDE->SetObjectDims(m_hObject, &vDims);
		}

		default : break;
	}

	return Powerup::EngineMessageFn(messageID, pData, lData);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	UnlockKey::AddPowerup
//
//	PURPOSE:	Add powerup to object
//
// ----------------------------------------------------------------------- //

void UnlockKey::AddPowerup(HOBJECT hObject)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	HMESSAGEWRITE hMessage = pServerDE->StartMessageToObject(this, hObject, MID_PICKUPKEY);
	pServerDE->WriteToMessageString(hMessage, pServerDE->GetObjectName(pServerDE->ObjectToHandle(this)));
	pServerDE->EndMessage(hMessage);
}



