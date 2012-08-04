// ----------------------------------------------------------------------- //
//
// MODULE  : ObjectiveDB.h
//
// PURPOSE : Database interface for player objectives
//
// CREATED : 03/23/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __OBJECTIVEDB_H__
#define __OBJECTIVEDB_H__




BEGIN_DATABASE_CATEGORY( Objective, "Interface/Objective" )

	DEFINE_GETRECORDATTRIB( Objective, char const* );
	DEFINE_GETRECORDATTRIB( IconTexture, char const* );
	DEFINE_GETRECORDATTRIB( TextDisplay, HRECORD );

END_DATABASE_CATEGORY( );


#endif  // __OBJECTIVEDB_H__

