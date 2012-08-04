// ----------------------------------------------------------------------- //
//
// MODULE  : CClientWeaponSFX.h
//
// PURPOSE : CClientWeaponSFX - Definition
//
// CREATED : 2/22/98
//
// ----------------------------------------------------------------------- //

#ifndef __CLIENT_WEAPON_SFX_H__
#define __CLIENT_WEAPON_SFX_H__

#include "ClientWeaponSFX.h"
#include "cpp_server_de.h"
#include "ClientSFX.h"
#include "WeaponDefs.h"
#include "SFXMsgIds.h"

// ----------------------------------------------------------------------- //

class CClientWeaponSFX : public CClientSFX
{
	public :

		CClientWeaponSFX() {  }

		void Setup(DVector *source, DVector *dest, DVector *forward, DVector *normal, DDWORD nFX, DDWORD nExtras, WeaponFXExtras *ext);

	protected :

		DDWORD EngineMessageFn(DDWORD messageID, void *pData, DFLOAT lData);
};


inline void SendWeaponSFXMessage(DVector *source, DVector *dest, DVector *forward, DVector *normal, DDWORD nFX, DDWORD nExtras, WeaponFXExtras *ext)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if(!pServerDE || !source || !dest || !forward || !normal) return;


	HMESSAGEWRITE hMessage = pServerDE->StartInstantSpecialEffectMessage(source);
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



#endif // __CLIENT_WEAPON_SFX_H__
