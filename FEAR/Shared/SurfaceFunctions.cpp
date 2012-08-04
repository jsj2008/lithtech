// ----------------------------------------------------------------------- //
//
// MODULE  : SurfaceFunctions.cpp
//
// PURPOSE : Implementation of shared surface functions
//
// CREATED : 7/13/99
//
// (c) 1999-2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "ContainerCodes.h"
#include "SurfaceFunctions.h"
#include "ClientServerShared.h"
#include "CommonUtilities.h"
#include "iltphysicssim.h" // for HPHYSICSRIGIDBODY
#include "iltcsbase.h"	   // for ILTCSBase


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ShowsClientFX
//
//	PURPOSE:	Does this type of surface show client fx
//
// ----------------------------------------------------------------------- //

bool ShowsClientFX(SurfaceType eSurfType)
{
	HRECORD hSurf = g_pSurfaceDB->GetSurface(eSurfType);
	if (hSurf)
	{
		return g_pSurfaceDB->GetBool(hSurf,SrfDB_Srf_bShowsFX);
	}

    return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ShowsTracks
//
//	PURPOSE:	Does this type of surface show tracks
//
// ----------------------------------------------------------------------- //

bool ShowsTracks(SurfaceType eSurfType)
{
	HRECORD hSurf = g_pSurfaceDB->GetSurface(eSurfType);
	if (hSurf)
	{
		return ShowsTracks(hSurf);
	}
	return false;
}
bool ShowsTracks(HSURFACE hSurf)
{
	if (hSurf)
	{
		const char* pszLeft = g_pSurfaceDB->GetString(hSurf,SrfDB_Srf_sLtFootPrintFX);
		const char* pszRight = g_pSurfaceDB->GetString(hSurf,SrfDB_Srf_sRtFootPrintFX);

		return (	(pszLeft && !LTStrEmpty(pszLeft)) || 
					(pszRight && !LTStrEmpty(pszRight))
				);
	}

    return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	UserFlagToSurface()
//
//	PURPOSE:	Convert a user flag to a surface type
//
// ----------------------------------------------------------------------- //

SurfaceType UserFlagToSurface(uint32 dwUserFlag)
{
	// Top byte contains surface flag

	SurfaceType eSurfType = (SurfaceType) (dwUserFlag >> 24);

	return eSurfType;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SurfaceToUserFlag()
//
//	PURPOSE:	Convert surface type to a user flag
//
// ----------------------------------------------------------------------- //

uint32 SurfaceToUserFlag(SurfaceType eSurfType)
{
	// Top byte should contain surface flag

    uint32 dwUserFlag = (uint32)eSurfType;
	dwUserFlag = (dwUserFlag << 24);

	return dwUserFlag;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	MovementSound()
//
//	PURPOSE:	Get movement sounds associated with this surface
//
// ----------------------------------------------------------------------- //

HRECORD MovementSound::GetSoundRecord( SurfaceType eSurfType, EFootSide eFootSide, ECharacterType eCharacterType )
{
	HRECORD hSurf = g_pSurfaceDB->GetSurface(eSurfType);

    LTASSERT(hSurf,"Invalid Surface specified");

    if (!hSurf) return NULL;

	char const* pszAttribute = NULL;

	switch( eCharacterType )
	{
	case eCharacterType_PlayerFirstPerson:
		pszAttribute = ( eFootSide == eFootSide_Left ) ? SrfDB_Srf_rLtPlayerFootSnd : SrfDB_Srf_rRtPlayerFootSnd;
		break;
	case eCharacterType_PlayerThirdPerson:
		pszAttribute = ( eFootSide == eFootSide_Left ) ? SrfDB_Srf_rLt3rdPlayerFootSnd : SrfDB_Srf_rRt3rdPlayerFootSnd;
		break;
	default:
	case eCharacterType_AI:
		pszAttribute = ( eFootSide == eFootSide_Left ) ? SrfDB_Srf_rLtDefaultFootSnd : SrfDB_Srf_rRtDefaultFootSnd;
		break;
	}

	HRECORD hSnd =  g_pSurfaceDB->GetRecordLink(hSurf,pszAttribute);
	return hSnd;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GetLandingSound()
//
//	PURPOSE:	Get landing sounds associated with this surface
//
// ----------------------------------------------------------------------- //

HRECORD GetLandingSound(SurfaceType eSurfType,
						 PlayerPhysicsModel eModel, bool bIsPlayer,
						 int32 PercentageOfDamageDistance0To100 )
{
	HRECORD hSurf = g_pSurfaceDB->GetSurface(eSurfType);
	HRECORD hSound=NULL;

	if (!hSurf) return NULL;

	switch (eModel)
	{
		// for now, no separation based on eModel or bIsPlayer.. but
		// it might be added later!
	case PPM_NORMAL :
	default :
		{
			int32 NumSounds;
			HATTRIBUTE hLandSoundList;

			// get the Landing sound struct and num values
			hLandSoundList = g_pSurfaceDB->GetLandingSoundStruct(hSurf);
			NumSounds = g_pLTDatabase->GetNumValues(hLandSoundList);

			if (NumSounds == 1)
			{
				// only one so use it.
				hSound = g_pSurfaceDB->GetLandingSoundStructSound(hLandSoundList, 0);
			}
			else if (NumSounds > 1)
			{
				// more than one, so iterate through the list until
				// there's one within the range. The range is implicitly
				// increasing, so the first value 'n' will cover from 0 to 'n',
				// the second from 'n'+1 to 'm', etc.
				// If not in range, use the last one (ie if '100' isn't
				// listed, or the percent is greater than 100, etc).
				// -- Terry

				int32 percent;
				bool bFound=false;
				int32 i;
				percent = LTCLAMP(PercentageOfDamageDistance0To100, 0, 100);

				for (i=0; (i < NumSounds) && !bFound; i++)
				{
					if (percent < g_pSurfaceDB->GetLandingSoundStructRange(hLandSoundList, i))
					{
						hSound = g_pSurfaceDB->GetLandingSoundStructSound(hLandSoundList, i);
						bFound=true;
					}
				}

				if (!bFound)
				{
					// just use the last one in the list if none is found.
					hSound = g_pSurfaceDB->GetLandingSoundStructSound(hLandSoundList, NumSounds-1);
				}
			}
		}
		break;
	}

	return hSound;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	IsMoveable()
//
//	PURPOSE:	Determine if the passed in object is moveable
//
// ----------------------------------------------------------------------- //

bool IsMoveable(HOBJECT hObj)
{
    if (!hObj) return false;

    uint32 dwUserFlags;
	g_pCommonLT->GetObjectFlags(hObj, OFT_User, dwUserFlags);

	return !!(dwUserFlags & USRFLG_MOVEABLE);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CanMarkObject()
//
//	PURPOSE:	Determine if the passed in object can be marked
//
// ----------------------------------------------------------------------- //

bool CanMarkObject(HOBJECT hObj)
{
    if (!hObj) return false;

	// Objects that don't move can be marked...

    if (!IsMoveable(hObj)) return true;

	SERVER_CODE
	(
		// Can also mark world models on the server...

		if (GetObjectType(hObj) == OT_WORLDMODEL)
		{
			return true;
		}
	)

    return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GetSurfaceType()
//
//	PURPOSE:	Determine the type of surface associated with the info
//				returned from an intersection...
//
// ----------------------------------------------------------------------- //

SurfaceType GetSurfaceType(IntersectInfo & iInfo)
{
	SurfaceType eType = ST_UNKNOWN;

	// Get the flags from the object, if none is specified, fall through
	// and try and get it from the polygon...

	if (iInfo.m_hObject)
	{
		eType = GetSurfaceType(iInfo.m_hObject);
	}

	if (iInfo.m_hPoly != INVALID_HPOLY && (eType == ST_UNKNOWN))
	{
		eType = GetSurfaceType(iInfo.m_hPoly);
	}

	return eType;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GetSurfaceType()
//
//	PURPOSE:	Determine the type of surface associated with the info
//				returned from a collision...
//
// ----------------------------------------------------------------------- //

SurfaceType GetSurfaceType(CollisionInfo & iInfo)
{
	SurfaceType eType = ST_UNKNOWN;

	// Get the flags from the object, if none is specified, fall through
	// and try and get it from the polygon...

	if (iInfo.m_hObject)
	{
		eType = GetSurfaceType(iInfo.m_hObject);
	}

	if (iInfo.m_hPoly != INVALID_HPOLY && (eType == ST_UNKNOWN))
	{
		eType = GetSurfaceType(iInfo.m_hPoly);
	}

	return eType;
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
	SurfaceType eType = ST_UNKNOWN;

	if (hPoly != INVALID_HPOLY)
	{
		// Get the flags associated with the poly...

        uint32 dwTextureFlags;

        g_pCommonLT->GetPolyTextureFlags(hPoly, &dwTextureFlags);

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

SurfaceType GetSurfaceType(HOBJECT hObj)
{
	if (!hObj) return ST_UNKNOWN;

	SurfaceType eType = ST_UNKNOWN;

    if (!IsMainWorld(hObj))
	{
		uint32 dwUserFlags;
		g_pCommonLT->GetObjectFlags(hObj, OFT_User, dwUserFlags);

		eType = UserFlagToSurface(dwUserFlags);
	}

	return eType;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GetSurfaceType()
//
//	PURPOSE:	Determine the type of surface associated with a rigid body
//
// ----------------------------------------------------------------------- //

SurfaceType GetSurfaceType(HPHYSICSRIGIDBODY hBody)
{
	// Need a valid hBody.  A NULL hBody represents the main world...
	if (!hBody) return ST_UNKNOWN;

	ILTCSBase* pLTBase = NULL;

	ILTPhysicsSim* pPhysicsSim = g_pLTBase->PhysicsSim();
	if (!pPhysicsSim) return ST_UNKNOWN;

	// For now the only way we can get the surface type from an hBody is
	// to get it from the object associated with the hBody (hopefully
	// we'll be able to do a 1 to 1 mapping in the future)...

	HOBJECT hObj = NULL;
	if (pPhysicsSim->GetRigidBodyObject(hBody, hObj) == LT_OK)
	{
		return GetSurfaceType(hObj);
	}

	return ST_UNKNOWN;
}

