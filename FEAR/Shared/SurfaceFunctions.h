// ----------------------------------------------------------------------- //
//
// MODULE  : SurfaceFunctions.h
//
// PURPOSE : Definition of shared surface functions
//
// CREATED : 4/16/98
//
// (c) 1998-2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __SURFACE_FUNCTIONS_H__
#define __SURFACE_FUNCTIONS_H__

#include "ltbasedefs.h"
#include "SurfaceDB.h"
#include "SharedMovement.h"

#ifndef __ILTPHYSICSSIM_H__
#include "iltphysicssim.h"
#endif

// forward declaration (defined in iltphysicssim.h)...
//class HPHYSICSRIGIDBODY;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ShowsClientFX
//
//	PURPOSE:	Does this type of surface show client fx
//
// ----------------------------------------------------------------------- //

bool ShowsClientFX(SurfaceType eSurfType);

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ShowsTracks
//
//	PURPOSE:	Does this type of surface show tracks
//
// ----------------------------------------------------------------------- //

bool ShowsTracks(SurfaceType eSurfType);
bool ShowsTracks(HSURFACE hSurf);

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	UserFlagToSurface()
//
//	PURPOSE:	Convert a user flag to a surface type
//
// ----------------------------------------------------------------------- //

SurfaceType UserFlagToSurface(uint32 dwUserFlag);

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SurfaceToUserFlag()
//
//	PURPOSE:	Convert surface type to a user flag
//
// ----------------------------------------------------------------------- //

uint32 SurfaceToUserFlag(SurfaceType eSurfType);

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	MovementSound()
//
//	PURPOSE:	Get movement sounds associated with this surface
//
// ----------------------------------------------------------------------- //
struct MovementSound
{
	enum EFootSide
	{
		eFootSide_Left,
		eFootSide_Right,
	};

	enum ECharacterType
	{
		eCharacterType_PlayerFirstPerson,
		eCharacterType_PlayerThirdPerson,
		eCharacterType_AI,
	};

	static HRECORD GetSoundRecord( SurfaceType eSurfType, EFootSide eFootSide, ECharacterType eCharacterType );
};

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GetLandingSound()
//
//	PURPOSE:	Get landing sounds associated with this surface
//
// ----------------------------------------------------------------------- //

HRECORD GetLandingSound(SurfaceType eSurfType,
						PlayerPhysicsModel eModel, bool bIsPlayer,
						int32 PercentageOfDamageDistance0To100 );

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	IsMoveable()
//
//	PURPOSE:	Determine if the passed in object is moveable
//
// ----------------------------------------------------------------------- //

bool IsMoveable(HOBJECT hObj);

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CanMarkObject()
//
//	PURPOSE:	Determine if the passed in object can be marked
//
// ----------------------------------------------------------------------- //

bool CanMarkObject(HOBJECT hObj);

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GetSurfaceType()
//
//	PURPOSE:	Determine the type of surface associated with the info
//				returned from an intersection...
//
// ----------------------------------------------------------------------- //

SurfaceType GetSurfaceType(IntersectInfo & iInfo);

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GetSurfaceType()
//
//	PURPOSE:	Determine the type of surface associated with the info
//				returned from a collision...
//
// ----------------------------------------------------------------------- //

SurfaceType GetSurfaceType(CollisionInfo & iInfo);

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GetSurfaceType()
//
//	PURPOSE:	Determine the type of surface associated with a poly
//
// ----------------------------------------------------------------------- //

SurfaceType GetSurfaceType(HPOLY hPoly);

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GetSurfaceType()
//
//	PURPOSE:	Determine the type of surface associated with an object
//
// ----------------------------------------------------------------------- //

SurfaceType GetSurfaceType(HOBJECT hObj);

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GetSurfaceType()
//
//	PURPOSE:	Determine the type of surface associated with a rigid body
//
// ----------------------------------------------------------------------- //

SurfaceType GetSurfaceType(HPHYSICSRIGIDBODY hBody);

#endif // __SURFACE_FUNCTIONS_H__
