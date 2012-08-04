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

#include "iltserver.h"

struct CLIENTWEAPONFX
{
	CLIENTWEAPONFX( )
	:	hFiredFrom			( INVALID_HOBJECT ),
		hObj				( INVALID_HOBJECT ),
		hWeapon				( NULL ),
		bSendWeaponRecord	( true ),
		hAmmo				( NULL ),
		bSendAmmoRecord		( true ),
		nSurfaceType		( ST_UNKNOWN ),
		wIgnoreFX			( 0 ),
		nShooterId			( (uint8)-1 ),
		bFXAtFlashSocket	( false ),
		vFirePos			( LTVector::GetIdentity( )),
		vPos				( LTVector::GetIdentity( )),
		vSurfaceNormal		( LTVector::GetIdentity( )),
		hNodeHit			( INVALID_MODEL_NODE )
	{ }

	HOBJECT			hFiredFrom;
	HOBJECT			hObj;
	HWEAPON			hWeapon;
	bool			bSendWeaponRecord;
	HAMMO			hAmmo;
	bool			bSendAmmoRecord;
	uint8			nSurfaceType;
	uint16			wIgnoreFX;
	uint8			nShooterId;
	bool			bFXAtFlashSocket;
	LTVector		vFirePos;
	LTVector		vPos;
	LTVector		vSurfaceNormal;
	HMODELNODE		hNodeHit;
};

void CreateClientWeaponFX(CLIENTWEAPONFX & theStruct);


#endif // __CLIENT_WEAPON_SFX_H__
