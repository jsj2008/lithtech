// ----------------------------------------------------------------------- //
//
// MODULE  : CursorsDB.h
//
// PURPOSE : Defines an interface for accessing cursor data
//
// CREATED : 06/24/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#pragma once
#ifndef __CURSORSDB_H__
#define __CURSORSDB_H__

#include "CategoryDB.h"

BEGIN_DATABASE_CATEGORY( Cursors, "Interface/Cursors" )

	//==========
	// Root.
	//==========
	DEFINE_GETRECORDATTRIB( HardwareCursor, char const* );
	DEFINE_GETRECORDATTRIB( SoftwareCursor, char const* );
	DEFINE_GETRECORDATTRIB( SoftwareCursorSize, LTVector2 );

END_DATABASE_CATEGORY( );



#endif  // __CURSORSDB_H__
