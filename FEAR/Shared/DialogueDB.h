// ----------------------------------------------------------------------- //
//
// MODULE  : DialogueDB.h
//
// PURPOSE : Defines interface to dialogue DB category
//
// CREATED : 11/16/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __DIALOGUEDB_H__
#define __DIALOGUEDB_H__

//
// Includes...
//

#include "CategoryDB.h"

// ----------------------------------------------------------------------- //
//
//	CLASS:		DialogueDB
//
//	PURPOSE:	Database for accessing Dialogue data
//
// ----------------------------------------------------------------------- //
BEGIN_DATABASE_CATEGORY( Dialogue, "Interface/Dialogue" )
	DEFINE_GETRECORDATTRIB( IconTexture, char const* );
	DEFINE_GETRECORDATTRIB( HeaderString, char const* );
	DEFINE_GETRECORDATTRIB( NameString, char const* );
END_DATABASE_CATEGORY( );




#endif  // __DIALOGUEDB_H__
