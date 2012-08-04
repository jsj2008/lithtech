// ----------------------------------------------------------------------- //
//
// MODULE  : DamageFxDB.h
//
// PURPOSE : Defines an interface for accessing damage\fx data.
//
// CREATED : 5/1/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __DAMAGEFXDB_H__
#define __DAMAGEFXDB_H__

//
// Includes...
//

#include "CategoryDB.h"

// ----------------------------------------------------------------------- //
//
//	CLASS:		DamageFxDB
//
//	PURPOSE:	Database for accessing SlowMo data
//
// ----------------------------------------------------------------------- //

BEGIN_DATABASE_CATEGORY( DamageFxDB, "Damage/FX" )
	DEFINE_GETRECORDATTRIB( DamageType, HRECORD );
	DEFINE_GETRECORDATTRIB( StartSound, char const* );
	DEFINE_GETRECORDATTRIB( LoopSound, char const* );
	DEFINE_GETRECORDATTRIB( FadeTime, int32 );	
	DEFINE_GETRECORDATTRIB( RotationSpeed, float );
	DEFINE_GETRECORDATTRIB( RotationMax, float );
	DEFINE_GETRECORDATTRIB( MinFXPercent, float );
	DEFINE_GETRECORDATTRIB( FOVMax, float );
	DEFINE_GETRECORDATTRIB( FOVXSpeed, float );
	DEFINE_GETRECORDATTRIB( FOVYSpeed, float );
	DEFINE_GETRECORDATTRIB( AllowMovement, bool );
	DEFINE_GETRECORDATTRIB( AllowInput, bool );
	DEFINE_GETRECORDATTRIB( NumJumpsToEscape, int32 );
	DEFINE_GETRECORDATTRIB( FirstPersonFX, char const* );
	DEFINE_GETRECORDATTRIB( ThirdPersonFX, char const* );
	DEFINE_GETRECORDATTRIB( FirstPersonInstantFX, char const* );
	DEFINE_GETRECORDATTRIB( ThirdPersonInstantFX, char const* );
	DEFINE_GETRECORDATTRIB( FirstPersonDeathFX, char const* );
	DEFINE_GETRECORDATTRIB( ThirdPersonDeathFX, char const* );
	DEFINE_GETRECORDATTRIB( BodyFX, char const* );
	DEFINE_GETRECORDATTRIB( TakingHealthFXName, char const* );
	DEFINE_GETRECORDATTRIB( TakingArmorFXName, char const* );
	DEFINE_GETRECORDATTRIB( AttachCameraToAni, bool );
	DEFINE_GETRECORDATTRIB( ShowClientModel, bool );
	DEFINE_GETRECORDATTRIB( InstantEffect, bool );
	DEFINE_GETRECORDATTRIB( AnimationControlsFX, bool );
	DEFINE_GETRECORDATTRIB( RemoveOnNextInstantDamage, bool );
	DEFINE_GETRECORDATTRIB( RenewOnNextInstantDamage, bool );
END_DATABASE_CATEGORY( );

#endif // __DAMAGEFXDB_H__
