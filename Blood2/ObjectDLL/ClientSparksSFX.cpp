// ----------------------------------------------------------------------- //
//
// MODULE  : CClientSparksSFX.cpp
//
// PURPOSE : CClientSparksSFX - Implementation
//
// CREATED : 1/17/98
//
// ----------------------------------------------------------------------- //

#include "ClientSparksSFX.h"
#include "cpp_server_de.h"
#include "SFXMsgIds.h"


BEGIN_CLASS(CClientSparksSFX)
END_CLASS_DEFAULT_FLAGS(CClientSparksSFX, CClientSFX, NULL, NULL, CF_HIDDEN)


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientSparksSFX::Setup
//
//	PURPOSE:	Send message to client with data
//
// ----------------------------------------------------------------------- //

void CClientSparksSFX::Setup(DVector *pvDir, DVector *pvColor1, DVector *pvColor2,
							 char* pFile, DBYTE nSparks, DFLOAT fDuration, DFLOAT fEmissionRadius,
 							 DFLOAT fParticleRadius, DFLOAT fGravity)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !pvDir || !pFile) return;

	HSTRING	hstrTexture = pServerDE->CreateString(pFile);

	// Use the object's pos and rotation...

	DVector vPos;
	VEC_INIT(vPos);
	DRotation rRot;
	ROT_INIT(rRot);

	// Tell the clients about the Sparks...

	HMESSAGEWRITE hMessage = pServerDE->StartInstantSpecialEffectMessage(&vPos);
	pServerDE->WriteToMessageByte(hMessage, SFX_SPARKS_ID);
//pServerDE->WriteToMessageByte(hMessage, DFALSE);
	pServerDE->WriteToMessageRotation(hMessage, &rRot);
	pServerDE->WriteToMessageVector(hMessage, &vPos);
	pServerDE->WriteToMessageVector(hMessage, pvDir);
	pServerDE->WriteToMessageVector(hMessage, pvColor1);
	pServerDE->WriteToMessageVector(hMessage, pvColor2);
	pServerDE->WriteToMessageHString(hMessage, hstrTexture);
	pServerDE->WriteToMessageByte(hMessage, nSparks);
	pServerDE->WriteToMessageFloat(hMessage, fDuration);
	pServerDE->WriteToMessageFloat(hMessage, fEmissionRadius);
	pServerDE->WriteToMessageFloat(hMessage, fParticleRadius);
	pServerDE->WriteToMessageFloat(hMessage, fGravity);
	pServerDE->EndMessage(hMessage);

	pServerDE->FreeString(hstrTexture);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientSparksSFX::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

DDWORD CClientSparksSFX::EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return 0;

	switch(messageID)
	{
		case MID_INITIALUPDATE:
		{
			pServerDE->SetNextUpdate(m_hObject, .0001f);
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
