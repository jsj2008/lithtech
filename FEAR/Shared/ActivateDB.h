// ----------------------------------------------------------------------- //
//
// MODULE  : ActivateDB.h
//
// PURPOSE : The ActivateTypeMgr object
//
// CREATED : 7/16/02
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __ACTIVATEDB_H__
#define __ACTIVATEDB_H__

using namespace std;

//
// Includes...
//

#include "CategoryDB.h"

//
// Defines...
//

struct ACTIVATETYPE
{
	static const uint8 INVALID_ID = 255;

	enum State
	{
		eInvalid = -1,
		eOn	= 0,
		eOff,

		eMaxStates
	};
};

// ----------------------------------------------------------------------- //
//
//	CLASS:		ActivateDB
//
//	PURPOSE:	Database for accessing Activate data
//
// ----------------------------------------------------------------------- //

BEGIN_DATABASE_CATEGORY( Activate, "FX/ActivateFX" )
	DEFINE_GETRECORDSTRUCT( States );
	DEFINE_GETSTRUCTATTRIB( States, HudText, const char * );
	DEFINE_GETSTRUCTATTRIB( States, HudIcon, const char * );
	DEFINE_GETRECORDATTRIB( DisabledIcon, const char * );
	DEFINE_GETRECORDATTRIB( DisabledColor, uint32 );
END_DATABASE_CATEGORY();

#endif // __ACTIVATEDB_H__
