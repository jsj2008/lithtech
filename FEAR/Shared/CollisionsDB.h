// ----------------------------------------------------------------------- //
//
// MODULE  : CollisionsDB.h
//
// PURPOSE : Defines an interface for accessing collisions data
//
// CREATED : 07/16/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __COLLISIONSDB_H__
#define __COLLISIONSDB_H__

#include "CategoryDB.h"

// ----------------------------------------------------------------------- //
//
//	CLASS:		CollisionsDB
//
//	PURPOSE:	Database for accessing Collisions data
//
// ----------------------------------------------------------------------- //


BEGIN_DATABASE_CATEGORY( Collisions, "Collisions" )

	HRECORD GetGlobalRecord( ) const { return g_pLTDatabase->GetRecord( GetCategory( ), "Global" ); }

	DEFINE_GETRECORDATTRIB( Default, HRECORD );
	DEFINE_GETRECORDATTRIB( MinVelocity, float );
	DEFINE_GETRECORDATTRIB( MaxPairs, uint32 );
	DEFINE_GETRECORDATTRIB( MinImpulse, uint32 );
	DEFINE_GETRECORDATTRIB( MinVolume, uint32 );
	DEFINE_GETRECORDATTRIB( MaxSounds, uint32 );
	DEFINE_GETRECORDATTRIB( SettleTime, float );

END_DATABASE_CATEGORY( );


// ----------------------------------------------------------------------- //
//
//	CLASS:		CollisionPropertyDB
//
//	PURPOSE:	Database for accessing CollisionProperty data
//
// ----------------------------------------------------------------------- //



BEGIN_DATABASE_CATEGORY( CollisionProperty, "Collisions/CollisionProperty" )

	//==========
	// Root.
	//==========
	DEFINE_GETRECORDATTRIB( Duration, float );
	DEFINE_GETRECORDATTRIB( Hardness, float );
	DEFINE_GETRECORDSTRUCT( Responses );

	//==========
	// Response.
	//==========
	DEFINE_GETSTRUCTATTRIB( Responses, WhenHitBy, HRECORD );
	DEFINE_GETSTRUCTSTRUCT( Responses, SoundRecord );
	DEFINE_GETSTRUCTSTRUCT( Responses, SoundVolume );
	DEFINE_GETSTRUCTSTRUCT( Responses, ClientFX );
	DEFINE_GETSTRUCTSTRUCT( Responses, AIStimulusRecord );
	DEFINE_GETSTRUCTSTRUCT( Responses, AIStimulusRadius );
	DEFINE_GETSTRUCTSTRUCT( Responses, Damage );

	//==========
	// Sound Record Response
	//==========
	DEFINE_GETSTRUCTATTRIB( SoundRecord, Impulse, uint32 );
	DEFINE_GETSTRUCTATTRIB( SoundRecord, Sound, HRECORD );

	//==========
	// Sound Volume Response
	//==========
	DEFINE_GETSTRUCTATTRIB( SoundVolume, Impulse, uint32 );
	DEFINE_GETSTRUCTATTRIB( SoundVolume, Volume, uint32 );

	//==========
	// ClientFX Response
	//==========
	DEFINE_GETSTRUCTATTRIB( ClientFX, Impulse, uint32 );
	DEFINE_GETSTRUCTATTRIB( ClientFX, ClientFX, char const* );
	DEFINE_GETSTRUCTATTRIB( ClientFX, Attached, bool );

	//==========
	// AI Stimulus Record Response
	//==========
	DEFINE_GETSTRUCTATTRIB( AIStimulusRecord, Impulse, uint32 );
	DEFINE_GETSTRUCTATTRIB( AIStimulusRecord, Stimulus, HRECORD );

	//==========
	// AI Stimulus Radius Response
	//==========
	DEFINE_GETSTRUCTATTRIB( AIStimulusRadius, Impulse, uint32 );
	DEFINE_GETSTRUCTATTRIB( AIStimulusRadius, Radius, uint32 );

	//==========
	// Damage 
	//==========
	DEFINE_GETSTRUCTATTRIB( Damage, DamageWho, char const* );
	DEFINE_GETSTRUCTSTRUCT( Damage, DamageResponses );

	//==========
	// Damage Response
	//==========
	DEFINE_GETSTRUCTATTRIB( DamageResponses, Impulse, uint32 );
	DEFINE_GETSTRUCTATTRIB( DamageResponses, DamageType, HRECORD );
	DEFINE_GETSTRUCTATTRIB( DamageResponses, Amount, float );

END_DATABASE_CATEGORY( );

#endif // __COLLISIONSDB_H__
