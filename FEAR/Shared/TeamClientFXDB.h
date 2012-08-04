// ----------------------------------------------------------------------- //
//
// MODULE  : TeamClientFXDB.h
//
// PURPOSE : The database interfaces for TeamClientFX categories.
//
// CREATED : 5/25/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __TEAMCLIENTFXDB_H__
#define __TEAMCLIENTFXDB_H__

//
// Includes...
//

#include "CategoryDB.h"

//
// Defines...
//

// ----------------------------------------------------------------------- //
//
//	CLASS:		TeamClientFX
//
//	PURPOSE:	Database for accessing TeamClientFX data
//
// ----------------------------------------------------------------------- //

BEGIN_DATABASE_CATEGORY( TeamClientFX, "FX/TeamFX" )
	DEFINE_GETRECORDATTRIB( FriendlyClientFX, char const* );
	DEFINE_GETRECORDATTRIB( EnemyClientFX, char const* );
END_DATABASE_CATEGORY();

#endif // __TEAMCLIENTFXDB_H__
