// ----------------------------------------------------------------------- //
//
// MODULE  : CTFDB.h
//
// PURPOSE : The database interfaces for GameModes\CTF categories.
//
// CREATED : 5/04/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __CTFDB_H__
#define __CTFDB_H__

//
// Includes...
//

#include "CategoryDB.h"

//
// Defines...
//

// ----------------------------------------------------------------------- //
//
//	CLASS:		CTFFlagBase
//
//	PURPOSE:	Database for accessing CTFGlobal data
//
// ----------------------------------------------------------------------- //

BEGIN_DATABASE_CATEGORY( CTFFlagBase, "GameModes/CTF/FlagBase" )
	DEFINE_GETRECORDATTRIB( BaseProp, HRECORD );
	DEFINE_GETRECORDATTRIB( BaseSocket, char const* );
	DEFINE_GETRECORDATTRIB( BaseAniHasFlag, char const* );
	DEFINE_GETRECORDATTRIB( BaseAniNoFlag, char const* );
	DEFINE_GETRECORDATTRIB( BaseTeamClientFxHasFlag, HRECORD );
	DEFINE_GETRECORDATTRIB( BaseTeamClientFxNoFlag, HRECORD );
	DEFINE_GETRECORDATTRIB( FlagProp, HRECORD );
	DEFINE_GETRECORDATTRIB( FlagCarrierSocket, char const* );
	DEFINE_GETRECORDATTRIB( FlagAniInBase, char const* );
	DEFINE_GETRECORDATTRIB( FlagAniCarried, char const* );
	DEFINE_GETRECORDATTRIB( FlagAniLoose, char const* );
	DEFINE_GETRECORDATTRIB( FlagTeamClientFxInBase, HRECORD );
	DEFINE_GETRECORDATTRIB( FlagTeamClientFxCarried, HRECORD );
	DEFINE_GETRECORDATTRIB( FlagTeamClientFxLoose, HRECORD );
	DEFINE_GETRECORDATTRIB( FlagCollisionProperty, HRECORD );
	DEFINE_GETRECORDATTRIB( FriendlyBaseNavMarker, HRECORD );
	DEFINE_GETRECORDATTRIB( EnemyBaseNavMarker, HRECORD );
END_DATABASE_CATEGORY();


// ----------------------------------------------------------------------- //
//
//	CLASS:		CTFRules
//
//	PURPOSE:	Database for accessing CTFRules data
//
// ----------------------------------------------------------------------- //

BEGIN_DATABASE_CATEGORY( CTFRules, "GameModes/CTF/Rules" )
	DEFINE_GETRECORDATTRIB( TheyStoleFlagMessage, char const* );
	DEFINE_GETRECORDATTRIB( WeStoleFlagMessage, char const* );
	DEFINE_GETRECORDATTRIB( TheyDropppedFlagMessage, char const* );
	DEFINE_GETRECORDATTRIB( WeDropppedFlagMessage, char const* );
	DEFINE_GETRECORDATTRIB( TheyPickedUpFlagMessage, char const* );
	DEFINE_GETRECORDATTRIB( WePickedUpFlagMessage, char const* );
	DEFINE_GETRECORDATTRIB( TheyCapturedFlagMessage, char const* );
	DEFINE_GETRECORDATTRIB( WeCapturedFlagMessage, char const* );
	DEFINE_GETRECORDATTRIB( TheyReturnedFlagMessage, char const* );
	DEFINE_GETRECORDATTRIB( WeReturnedFlagMessage, char const* );
	DEFINE_GETRECORDATTRIB( AutoReturnedTheirFlagMessage, char const* );
	DEFINE_GETRECORDATTRIB( AutoReturnedOurFlagMessage, char const* );
	DEFINE_GETRECORDATTRIB( DefendFlagMessage, char const* );
	DEFINE_GETRECORDATTRIB( DefendFlagCarrierMessage, char const* );
	DEFINE_GETRECORDATTRIB( CaptureAssistMessage, char const* );
END_DATABASE_CATEGORY();

#endif // __CTFDB_H__
