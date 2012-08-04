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
#include <memory.h>  // for memset
#include "ImpactType.h"

struct CLIENTWEAPONFX
{
    CLIENTWEAPONFX();

    HOBJECT      hFiredFrom;
    HOBJECT      hObj;
    uint8        nWeaponId;
    uint8        nAmmoId;
    uint8        nSurfaceType;
    uint16       wIgnoreFX;
    uint8        nShooterId;
    LTVector     vFirePos;
    LTVector     vPos;
    LTVector     vSurfaceNormal;
    IMPACT_TYPE  eImpactType;
};

inline CLIENTWEAPONFX::CLIENTWEAPONFX()
{
	memset(this, 0, sizeof(CLIENTWEAPONFX));
	nShooterId = -1;
}


void CreateClientWeaponFX(CLIENTWEAPONFX & theStruct);


#endif // __CLIENT_WEAPON_SFX_H__