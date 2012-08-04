// ----------------------------------------------------------------------- //
//
// MODULE  : CClientWeaponSFX.cpp
//
// PURPOSE : CClientWeaponSFX - Implementation
//
// CREATED : 1/17/98
//
// ----------------------------------------------------------------------- //

#include "ClientWeaponSFX.h"
#include "cpp_server_de.h"
#include "SFXMsgIds.h"
#include "SharedDefs.h"

BEGIN_CLASS(CClientWeaponSFX)
END_CLASS_DEFAULT_FLAGS(CClientWeaponSFX, CClientSFX, NULL, NULL, CF_HIDDEN)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeaponSFX::Setup
//
//	PURPOSE:	Send message to client with data
//
// ----------------------------------------------------------------------- //

void CClientWeaponSFX::Setup(DVector *source, DVector *dest, DVector *forward, DVector *normal, DDWORD nFX, DDWORD nExtras, WeaponFXExtras *ext)
{
	CServerDE* pServerDE = GetServerDE();
	if(!pServerDE || !source || !dest || !forward || !normal) return;

	HMESSAGEWRITE hMessage = pServerDE->StartSpecialEffectMessage(this);
	pServerDE->WriteToMessageByte(hMessage, SFX_WEAPON_ID);

	pServerDE->WriteToMessageVector(hMessage, source);
	pServerDE->WriteToMessageVector(hMessage, dest);
	pServerDE->WriteToMessageVector(hMessage, forward);
	pServerDE->WriteToMessageVector(hMessage, normal);
	pServerDE->WriteToMessageDWord(hMessage, nFX);
	pServerDE->WriteToMessageDWord(hMessage, nExtras);

	if(nExtras & WFX_EXTRA_AMMOTYPE)
		pServerDE->WriteToMessageByte(hMessage, ext->nAmmo);
	if(nExtras & WFX_EXTRA_SURFACETYPE)
		pServerDE->WriteToMessageByte(hMessage, ext->nSurface);
	if(nExtras & WFX_EXTRA_EXPTYPE)
		pServerDE->WriteToMessageByte(hMessage, ext->nExp);
	if(nExtras & WFX_EXTRA_DAMAGE)
		pServerDE->WriteToMessageFloat(hMessage, ext->fDamage);
	if(nExtras & WFX_EXTRA_DENSITY)
		pServerDE->WriteToMessageFloat(hMessage, ext->fDensity);

	if(nExtras & WFX_EXTRA_COLOR1)
		pServerDE->WriteToMessageVector(hMessage, &(ext->vColor1));
	if(nExtras & WFX_EXTRA_COLOR2)
		pServerDE->WriteToMessageVector(hMessage, &(ext->vColor2));
	if(nExtras & WFX_EXTRA_LIGHTCOLOR1)
		pServerDE->WriteToMessageVector(hMessage, &(ext->vLightColor1));
	if(nExtras & WFX_EXTRA_LIGHTCOLOR2)
		pServerDE->WriteToMessageVector(hMessage, &(ext->vLightColor2));

	pServerDE->EndMessage(hMessage);
}

// ----------------------------------------------------------------------- //

DDWORD CClientWeaponSFX::EngineMessageFn(DDWORD messageID, void *pData, DFLOAT lData)
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

	return CClientSFX::EngineMessageFn(messageID, pData, lData);
}
