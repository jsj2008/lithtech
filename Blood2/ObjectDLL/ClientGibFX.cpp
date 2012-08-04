// ----------------------------------------------------------------------- //
//
// MODULE  : ClientGibFX.cpp
//
// PURPOSE : CClientGibFX - Implementation
//
// CREATED : 8/3/98
//
// ----------------------------------------------------------------------- //

#include "ClientGibFX.h"
#include "cpp_server_de.h"
#include "SFXMsgIds.h"

/*
BEGIN_CLASS(CClientGibFX)
END_CLASS_DEFAULT_FLAGS(CClientGibFX, CClientSFX, NULL, NULL, CF_HIDDEN)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientGibFX::Setup
//
//	PURPOSE:	Send message to client with data
//
// ----------------------------------------------------------------------- //

void CClientGibFX::Setup( DVector *pvPos, DVector *pvDir, DVector *pvDims, DDWORD dwFlags, DFLOAT fScale, DBYTE nCount,
						 HSTRING hstrModel, HSTRING hstrTexture, HSTRING hstrSound)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	VEC_NEGATE(*pvDir, *pvDir);

	HMESSAGEWRITE hMessage = pServerDE->StartInstantSpecialEffectMessage( pvPos );
	pServerDE->WriteToMessageByte(hMessage, SFX_GIB_ID);
	pServerDE->WriteToMessageCompPosition(hMessage, pvPos);
	pServerDE->WriteToMessageCompVector(hMessage, pvDir);
	pServerDE->WriteToMessageCompVector(hMessage, pvDims);
	pServerDE->WriteToMessageWord(hMessage, (D_WORD)dwFlags);
	pServerDE->WriteToMessageFloat(hMessage, fScale);
	pServerDE->WriteToMessageByte(hMessage, nCount);

	if (dwFlags & TYPEFLAG_CUSTOM)
	{
		pServerDE->WriteToMessageHString(hMessage, hstrModel);
		pServerDE->WriteToMessageHString(hMessage, hstrTexture);
		pServerDE->WriteToMessageHString(hMessage, hstrSound);
	}
	
	pServerDE->EndMessage(hMessage);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientGibFX::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

DDWORD CClientGibFX::EngineMessageFn(DDWORD messageID, void *pData, DFLOAT lData)
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
*/


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SetutClientGibFX
//
//	PURPOSE:	Send message to client with data
//
// ----------------------------------------------------------------------- //

void SetupClientGibFX( DVector *pvPos, DVector *pvDir, DVector *pvDims, DDWORD dwFlags, DFLOAT fScale, DBYTE nCount,
						 HSTRING hstrModel, HSTRING hstrTexture, HSTRING hstrSound)
{
	if (!g_pServerDE) return;

	VEC_NEGATE(*pvDir, *pvDir);

	HMESSAGEWRITE hMessage = g_pServerDE->StartInstantSpecialEffectMessage( pvPos );
	g_pServerDE->WriteToMessageByte(hMessage, SFX_GIB_ID);
	g_pServerDE->WriteToMessageCompPosition(hMessage, pvPos);
	g_pServerDE->WriteToMessageCompVector(hMessage, pvDir);
	g_pServerDE->WriteToMessageCompVector(hMessage, pvDims);
	g_pServerDE->WriteToMessageWord(hMessage, (D_WORD)dwFlags);
	g_pServerDE->WriteToMessageFloat(hMessage, fScale);
	g_pServerDE->WriteToMessageByte(hMessage, nCount);

	if (dwFlags & TYPEFLAG_CUSTOM)
	{
		g_pServerDE->WriteToMessageHString(hMessage, hstrModel);
		g_pServerDE->WriteToMessageHString(hMessage, hstrTexture);
		g_pServerDE->WriteToMessageHString(hMessage, hstrSound);
	}
	
	g_pServerDE->EndMessage(hMessage);
}


