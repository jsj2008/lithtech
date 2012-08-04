// ----------------------------------------------------------------------- //
//
// MODULE  : SurfaceFunctions.h
//
// PURPOSE : Definition of server-side surface functions
//
// CREATED : 4/16/98
//
// ----------------------------------------------------------------------- //

#ifndef __SURFACE_FUNCTIONS_H__
#define __SURFACE_FUNCTIONS_H__

#include "SurfaceTypes.h"
#include "BaseCharacter.h"
#include "VolumeBrush.h"

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	IsMoveable()
//
//	PURPOSE:	Determine if the passed in object is moveable
//
// ----------------------------------------------------------------------- //

inline DBOOL IsMoveable(HOBJECT hObj)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE || !hObj) return DFALSE;

	DDWORD dwUserFlags = pServerDE->GetObjectUserFlags(hObj);
	return !!(dwUserFlags & USRFLG_MOVEABLE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GetSurfaceType()
//
//	PURPOSE:	Determine the type of surface associated with a poly
//
// ----------------------------------------------------------------------- //

inline SurfaceType GetSurfaceType(HPOLY hPoly)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE || !hPoly) return ST_UNKNOWN;

	SurfaceType eType = ST_UNKNOWN;

	if (hPoly != INVALID_HPOLY)
	{
		// Get the flags associated with the poly...

		DDWORD dwTextureFlags;
		pServerDE->GetPolyTextureFlags(hPoly, &dwTextureFlags);

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

inline SurfaceType GetSurfaceType(HOBJECT hObj)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE || !hObj) return ST_UNKNOWN;

	SurfaceType eType = ST_UNKNOWN;

	if (pServerDE->GetWorldObject() != hObj)
	{
		HCLASS hObjClass  = pServerDE->GetObjectClass(hObj);
		HCLASS hBase	  = pServerDE->GetClass("CBaseCharacter");
		HCLASS hVolume	  = pServerDE->GetClass("VolumeBrush");

		if (pServerDE->IsKindOf(hObjClass, hBase))
		{
			CBaseCharacter* pBase = (CBaseCharacter*)pServerDE->HandleToObject(hObj);
			if (pBase)
			{
				if (pBase->IsMecha() || IsVehicle(hObj))
				{
					eType = ST_MECHA;
				}
				else
				{
					eType = ST_FLESH;
				}
			}
		}
		else if (pServerDE->IsKindOf(hObjClass, hVolume))
		{
			VolumeBrush* pVB = (VolumeBrush*)pServerDE->HandleToObject(hObj);
			if (pVB)
			{
				if (IsLiquid(pVB->GetCode()))
				{
					eType = ST_LIQUID;
				}
			}
		}
		else 
		{
			DDWORD dwUserFlags = pServerDE->GetObjectUserFlags(hObj);
			eType = UserFlagToSurface(dwUserFlags);
		}
	}

	return eType;
}


#endif // __SURFACE_FUNCTIONS_H__