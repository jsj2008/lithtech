// ----------------------------------------------------------------------- //
//
// MODULE  : ClientWeaponUtils.h
//
// PURPOSE : Client-side firing helper functions
//
// CREATED : 11/2/98
//
// ----------------------------------------------------------------------- //

#ifndef __CLIENT_WEAPON_UTILS_H__
#define __CLIENT_WEAPON_UTILS_H__

#include "ltbasedefs.h"
#include "SurfaceMgr.h"

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AttackerLiquidFilterFn()
//
//	PURPOSE:	Filter the attacker out of CastRay and/or
//				IntersectSegment calls (so you don't shot yourself).
//				However, we want to ignore liquid as well...
//
// ----------------------------------------------------------------------- //

LTBOOL AttackerLiquidFilterFn(HLOCALOBJ hObj, void *pUserData);


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AddLocalImpactFX
//
//	PURPOSE:	Add a weapon impact special fx
//
// ----------------------------------------------------------------------- //

void AddLocalImpactFX(HLOCALOBJ hObj, LTVector & vFirePos, LTVector & vImpactPoint,
                      LTVector & vNormal, SurfaceType eType, LTVector & vPath,
                      uint8 nWeaponId, uint8 nAmmoId, uint16 wIgnoreFX);


#endif // __CLIENT_WEAPON_UTILS_H__