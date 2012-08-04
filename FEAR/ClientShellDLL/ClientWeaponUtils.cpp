// ----------------------------------------------------------------------- //
//
// MODULE  : ClientWeaponUtils.cpp
//
// PURPOSE : Client-side firing helper functions
//
// CREATED : 11/2/98
//
// (c) 1998-2005 Monolith Productions, Inc.  All Rights Reserved
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

bool AttackerLiquidFilterFn(HLOCALOBJ hObj, void *pUserData)
{
	// We're not attacking our self...

	if (SpecificObjectFilterFn(hObj, pUserData))
	{
        // Return true to keep this object (not liquid), or false to ignore
		// this object (is liquid)...

        uint16 code;
        if (g_pLTClient && g_pLTClient->GetContainerCode(hObj, &code))
		{
			ContainerCode eCode = (ContainerCode)code;

			if (IsLiquid(eCode))
			{
                return false;
			}
		}

        return true;
	}

    return false;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AddLocalImpactFX
//
//	PURPOSE:	Add a weapon impact special fx
//
// ----------------------------------------------------------------------- //

void AddLocalImpactFX( HLOCALOBJ hObj, const LTVector & vFirePos, const LTVector & vImpactPoint,
						const LTVector & vNormal, SurfaceType eType, const LTVector & vPath,
						HWEAPON hWeapon, HAMMO hAmmo, uint16 wIgnoreFX, bool bFXAtFlashSocket,
						HMODELNODE hHitNode )
{
	if (!g_pLTClient || !g_pGameClientShell) return;

	CSFXMgr* psfxMgr = g_pGameClientShell->GetSFXMgr();
	if (!psfxMgr) return;

	LTVector vPos;
	vPos = vImpactPoint - vPath;

	uint32 dwId;
	g_pLTClient->GetLocalClientID(&dwId);

	uint16 wIgnFX = (wIgnoreFX | WFX_IMPACTDING); // Never play ding on local impacts;

	if (!CanMarkObject(hObj))
	{
		wIgnFX |= WFX_MARK;
	}

	WCREATESTRUCT w;

	w.hObjectHit		= hObj;
	w.hWeapon			= hWeapon;
	w.hAmmo				= hAmmo;
	w.nSurfaceType		= eType;
	w.wIgnoreFX 		= wIgnFX;
	w.nShooterId		= (uint8)dwId;
	w.vFirePos			= vFirePos;
	w.vPos				= vPos;
	w.vSurfaceNormal	= vNormal;
	w.bFXAtFlashSocket	= bFXAtFlashSocket;
	w.hNodeHit			= hHitNode;
	
	psfxMgr->CreateSFX(SFX_WEAPON_ID, &w);
}
