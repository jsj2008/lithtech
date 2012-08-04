// ----------------------------------------------------------------------- //
//
// MODULE  : ClientWeaponUtils.cpp
//
// PURPOSE : Client-side firing helper functions
//
// CREATED : 11/2/98
//
// ----------------------------------------------------------------------- //

#include "ClientWeaponUtils.h"
#include "ClientUtilities.h"
#include "RiotClientShell.h"
#include "ClientServerShared.h"
#include "WeaponFX.h"
#include "iltmath.h"

extern CRiotClientShell* g_pRiotClientShell;
extern ILTClient* g_pClientDE;

static ILTMath* pMath;
define_holder(ILTMath, pMath);

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	IsMoveable()
//
//	PURPOSE:	Determine if the passed in object is moveable
//
// ----------------------------------------------------------------------- //

LTBOOL IsMoveable(HLOCALOBJ hObj)
{
	if (!g_pClientDE || !hObj) return LTFALSE;

	uint32 dwUserFlags;
	g_pClientDE->Common()->GetObjectFlags(hObj, OFT_User, dwUserFlags);

	return !!(dwUserFlags & USRFLG_MOVEABLE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GetSurfaceType()
//
//	PURPOSE:	Determine the type of surface associated with a poly
//
// ----------------------------------------------------------------------- //

SurfaceType GetSurfaceType(HPOLY hPoly)
{
	if (!g_pClientDE) return ST_UNKNOWN;

	SurfaceType eType = ST_UNKNOWN;

	if (hPoly != INVALID_HPOLY)
	{
		// Get the flags associated with the poly...

		uint32 dwTextureFlags;
		g_pClientDE->GetPolyTextureFlags(hPoly, &dwTextureFlags);

		eType = (SurfaceType)dwTextureFlags;
	}
	
	return eType;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GetSurfaceType()
//
//	PURPOSE:	Determine the type of surface associated with an object
//
// ----------------------------------------------------------------------- //

SurfaceType GetSurfaceType(HLOCALOBJ hObj)
{
	if (!g_pClientDE || !hObj) return ST_UNKNOWN;

	SurfaceType eType = ST_UNKNOWN;

	uint16 code;
	uint32 dwUserFlags;
	g_pClientDE->Common()->GetObjectFlags(hObj, OFT_User, dwUserFlags);

	if (dwUserFlags & USRFLG_CHARACTER)
	{
		eType = ST_FLESH;
	}
	else if (g_pClientDE->GetContainerCode(hObj, &code))
	{
		ContainerCode eCode = (ContainerCode)code;

		if (IsLiquid(eCode))
		{
			eType = ST_LIQUID;
		}
	}
	else 
	{
		eType = UserFlagToSurface(dwUserFlags);
	}

	return eType;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ObjListFilterFn()
//
//	PURPOSE:	Filter specific objects out of CastRay and/or 
//				IntersectSegment calls.
//
// ----------------------------------------------------------------------- //

LTBOOL ObjListFilterFn(HLOCALOBJ hTest, void *pUserData)
{
	HOBJECT *hList;

	// Filters out objects for a raycast.  pUserData is a list of HOBJECTS terminated
	// with a NULL HOBJECT.
	hList = (HOBJECT*)pUserData;
	while(*hList)
	{
		if(hTest == *hList)
			return LTFALSE;
		++hList;
	}
	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SpecificObjectFilterFn()
//
//	PURPOSE:	Filter a specific object out of CastRay and/or 
//				IntersectSegment calls.
//
// ----------------------------------------------------------------------- //

LTBOOL SpecificObjectFilterFn(HLOCALOBJ hObj, void *pUserData)
{
	if (!hObj) return LTFALSE;

	return (hObj != (HLOCALOBJ)pUserData);
}

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
		if (g_pClientDE && g_pClientDE->GetContainerCode(hObj, &code))
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
					  uint8 nWeaponId, uint8 nIgnoreFX)
{
	if (!g_pClientDE || !g_pRiotClientShell) return;

	CSFXMgr* psfxMgr = g_pRiotClientShell->GetSFXMgr();
	if (!psfxMgr) return;

	LTVector vPos, vTemp;
	VEC_COPY(vPos, vImpactPoint);
	VEC_MULSCALAR(vTemp, vPath, -1.0f);
	VEC_ADD(vPos, vPos, vTemp);

	LTRotation rRot;
	pMath->AlignRotation(rRot, vNormal, LTVector(0, 1, 0));

	uint32 dwId;
	g_pClientDE->GetLocalClientID(&dwId);

	uint8 nIgnFX = nIgnoreFX;

	if (IsMoveable(hObj))
	{
		nIgnFX |= WFX_MARK | WFX_SMOKE;	 
	}

	WCREATESTRUCT w;

	w.nWeaponId		= nWeaponId;
	w.nSurfaceType	= eType;
	w.nIgnoreFX 	= nIgnFX;
	w.nShooterId 	= (uint8)dwId;
	w.bLocal		= LTTRUE;

	VEC_COPY(w.vFirePos, vFirePos);
	VEC_COPY(w.vPos, vPos);
	w.rRot = rRot;

	psfxMgr->CreateSFX(SFX_WEAPON_ID, &w);
}


