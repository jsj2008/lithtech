// ----------------------------------------------------------------------- //
//
// MODULE  : GameModesDB.h
//
// PURPOSE : Database interface for GameModes
//
// CREATED : 09/08/04
//
// (c) 1999-2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef GAMMODESDB_H
#define GAMMODESDB_H

#include "CategoryDB.h"

// ----------------------------------------------------------------------- //
//
//	CLASS:		GameModesDB
//
//	PURPOSE:	Database for accessing GameModes data
//
// ----------------------------------------------------------------------- //


BEGIN_DATABASE_CATEGORY( GameModes, "GameModes" )

	DEFINE_GETRECORDATTRIB( Label, char const* );
	DEFINE_GETRECORDATTRIB( Multiplayer, bool );

END_DATABASE_CATEGORY( );


#endif // GAMMODESDB_H
