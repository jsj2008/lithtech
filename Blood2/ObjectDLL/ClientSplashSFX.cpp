// ----------------------------------------------------------------------- //
//
// MODULE  : ClientSplashSFX.cpp
//
// PURPOSE : CClientSplashSFX - Implementation
//
// CREATED : 1/17/98
//
// ----------------------------------------------------------------------- //

#include "ClientSplashSFX.h"
#include "cpp_server_de.h"
#include "SFXMsgIds.h"


BEGIN_CLASS(CClientSplashSFX)
END_CLASS_DEFAULT_FLAGS(CClientSplashSFX, CClientSFX, NULL, NULL, CF_HIDDEN)


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientSplashSFX::Setup
//
//	PURPOSE:	Send message to client with data
//
// ----------------------------------------------------------------------- //

void CClientSplashSFX::Setup(DVector *vPos, DVector *vDir, DFLOAT fRadius, DFLOAT fPosRadius, DFLOAT fHeight,
							 DFLOAT fDensity, DFLOAT fSpread, DVector *vColor1, DVector *vColor2, DVector *vRippleScale,
							 DBOOL bRipple, DFLOAT fSprayTime, DFLOAT fDuration, DFLOAT fGravity, char *pFile)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !vDir || !pFile) return;

	HSTRING	hstrTexture = pServerDE->CreateString(pFile);

	// Tell the clients about the Splash...
	HMESSAGEWRITE hMessage = pServerDE->StartInstantSpecialEffectMessage( vPos );
	pServerDE->WriteToMessageByte(hMessage, SFX_SPLASH_ID);
//pServerDE->WriteToMessageByte(hMessage, DFALSE);

	pServerDE->WriteToMessageVector(hMessage, vPos);
	pServerDE->WriteToMessageVector(hMessage, vDir);
	pServerDE->WriteToMessageFloat(hMessage, fRadius);
	pServerDE->WriteToMessageFloat(hMessage, fPosRadius);
	pServerDE->WriteToMessageFloat(hMessage, fHeight);
	pServerDE->WriteToMessageFloat(hMessage, fDensity);
	pServerDE->WriteToMessageFloat(hMessage, fSpread);
	pServerDE->WriteToMessageVector(hMessage, vColor1);
	pServerDE->WriteToMessageVector(hMessage, vColor2);
	pServerDE->WriteToMessageVector(hMessage, vRippleScale);
	pServerDE->WriteToMessageByte(hMessage, bRipple);
	pServerDE->WriteToMessageFloat(hMessage, fSprayTime);
	pServerDE->WriteToMessageFloat(hMessage, fDuration);
	pServerDE->WriteToMessageFloat(hMessage, fGravity);
	pServerDE->WriteToMessageHString(hMessage, hstrTexture);
	pServerDE->EndMessage(hMessage);

	pServerDE->FreeString(hstrTexture);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientSplashSFX::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

DDWORD CClientSplashSFX::EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData)
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