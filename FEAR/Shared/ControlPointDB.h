// ----------------------------------------------------------------------- //
//
// MODULE  : ControlPointDB.h
//
// PURPOSE : The database interfaces for GameModes\CP categories.
//
// CREATED : 02/09/06
//
// (c) 2006 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __CONTROLPOINTDB_H__
#define __CONTROLPOINTDB_H__

//
// Includes...
//

#include "CategoryDB.h"

//
// Defines...
//

// ----------------------------------------------------------------------- //
//
//	CLASS:		CPTypes
//
//	PURPOSE:	Database for accessing CTFGlobal data
//
// ----------------------------------------------------------------------- //

BEGIN_DATABASE_CATEGORY( CPTypes, "GameModes/CP/Types" )
	DEFINE_GETRECORDATTRIB( Prop, HRECORD );
	DEFINE_GETRECORDATTRIB( AniCaptured, char const* );
	DEFINE_GETRECORDATTRIB( AniNeutral, char const* );
	DEFINE_GETRECORDATTRIB( TeamClientFxCaptured, HRECORD );
	DEFINE_GETRECORDATTRIB( ClientFxNeutral, char const* );
	DEFINE_GETRECORDSTRUCT( Stages );
		DEFINE_GETSTRUCTATTRIB( Stages, TeamClientFxCapturedStage, HRECORD );
		DEFINE_GETSTRUCTATTRIB( Stages, TeamClientFxNeutralStage, HRECORD );
	DEFINE_GETRECORDATTRIB( DefaultZoneDims, LTVector );
	DEFINE_GETRECORDATTRIB( Solid, bool );
	DEFINE_GETRECORDATTRIB( FriendlyNavMarker, HRECORD );
	DEFINE_GETRECORDATTRIB( EnemyNavMarker, HRECORD );
	DEFINE_GETRECORDATTRIB( NeutralNavMarker, HRECORD );
	DEFINE_GETRECORDATTRIB( CapturingSoundFX, HRECORD );
	DEFINE_GETRECORDATTRIB( AttractSpawns, bool );
	DEFINE_GETRECORDATTRIB( SurfaceType, HRECORD );
	DEFINE_GETRECORDATTRIB( CollisionProperty, HRECORD );
END_DATABASE_CATEGORY();


// ----------------------------------------------------------------------- //
//
//	CLASS:		CPRules
//
//	PURPOSE:	Database for accessing Control Point Rules data
//
// ----------------------------------------------------------------------- //

BEGIN_DATABASE_CATEGORY( CPRules, "GameModes/CP/Rules" )
	DEFINE_GETRECORDATTRIB( TheyCapturedCPMessage, char const* );
	DEFINE_GETRECORDATTRIB( WeCapturedCPMessage, char const* );
	DEFINE_GETRECORDATTRIB( TheyNeutralizedCPMessage, char const* );
	DEFINE_GETRECORDATTRIB( WeNeutralizedCPMessage, char const* );
	DEFINE_GETRECORDATTRIB( PlayerCaptureStringId, char const* );
	DEFINE_GETRECORDATTRIB( PlayerNeutralizeStringId, char const* );
	DEFINE_GETRECORDATTRIB( PlayerFixupStringId, char const* );
	DEFINE_GETRECORDATTRIB( PlayerDefendStringId, char const* );
	DEFINE_GETRECORDATTRIB( ConquestVictoryStringId, char const* );
END_DATABASE_CATEGORY();

#endif // __CONTROLPOINTDB_H__
