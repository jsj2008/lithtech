// ----------------------------------------------------------------------- //
//
// MODULE  : ClientWeaponUtils.cpp
//
// PURPOSE : Client-side firing helper functions
//
// CREATED : 11/2/98
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ClientWeaponUtils.h"
#include "ClientUtilities.h"
#include "GameClientShell.h"
#include "ClientServerShared.h"
#include "WeaponFX.h"
#include "WeaponFXTypes.h"
#include "SurfaceFunctions.h"

extern CGameClientShell* g_pGameClientShell;
extern ILTClient* g_pLTClient;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AttackerLiquidFilterFn()
//
//	PURPOSE:	Filter the attacker out of CastRay and/or
//				IntersectSegment calls (so you don't shot yourself).
//				However, we want to ignore liquid as well...
//
// ----------------------------------------------------------------------- //

LTBOOL AttackerLiquidFilterFn(HLOCALOBJ hObj, void *pUserData)
{
	// We're not attacking our self...

	if (SpecificObjectFilterFn(hObj, pUserData))
	{
        // Return LTTRUE to keep this object (not liquid), or LTFALSE to ignore
		// this object (is liquid)...

        uint16 code;
        if (g_pLTClient && g_pLTClient->GetContainerCode(hObj, &code))
		{
			ContainerCode eCode = (ContainerCode)code;

			if (IsLiquid(eCode))
			{
                return LTFALSE;
			}
		}

        return LTTRUE;
	}

    return LTFALSE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AddLocalImpactFX
//
//	PURPOSE:	Add a weapon impact special fx
//
// ----------------------------------------------------------------------- //

void AddLocalImpactFX(HLOCALOBJ hObj, LTVector & vFirePos, LTVector & vImpactPoint,
                      LTVector & vNormal, SurfaceType eType, LTVector & vPath,
                      uint8 nWeaponId, uint8 nAmmoId, uint16 wIgnoreFX)
{
    if (!g_pLTClient || !g_pGameClientShell) return;

	CSFXMgr* psfxMgr = g_pGameClientShell->GetSFXMgr();
	if (!psfxMgr) return;

    LTVector vPos, vTemp;
	VEC_COPY(vPos, vImpactPoint);
	VEC_MULSCALAR(vTemp, vPath, -1.0f);
	VEC_ADD(vPos, vPos, vTemp);

    uint32 dwId;
    g_pLTClient->GetLocalClientID(&dwId);

    uint16 wIgnFX = wIgnoreFX;

	if (!CanMarkObject(hObj))
	{
		wIgnFX |= WFX_MARK;
	}

	WCREATESTRUCT w;

	w.nWeaponId		= nWeaponId;
	w.nAmmoId		= nAmmoId;
	w.nSurfaceType	= eType;
	w.wIgnoreFX 	= wIgnFX;
    w.nShooterId    = (uint8)dwId;
    w.bLocal        = LTTRUE;

	w.vFirePos		 = vFirePos;
	w.vPos			 = vPos;
	w.vSurfaceNormal = vNormal;

	psfxMgr->CreateSFX(SFX_WEAPON_ID, &w);
}
