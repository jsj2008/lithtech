// ----------------------------------------------------------------------- //
//
// MODULE  : CreditsDB.h
//
// PURPOSE : Defines an interface for accessing SlowMo data
//
// CREATED : 11/09/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __CREDITSDB_H__
#define __CREDITSDB_H__

//
// Includes...
//

#include "CategoryDB.h"

// ----------------------------------------------------------------------- //
//
//	CLASS:		CreditsDB
//
//	PURPOSE:	Database for accessing SlowMo data
//
// ----------------------------------------------------------------------- //

BEGIN_DATABASE_CATEGORY( CreditEntry, "Interface/Credits/Entries" )
	DEFINE_GETRECORDATTRIB( BasePos, LTVector2 );
	DEFINE_GETRECORDATTRIB( TextAlignment, char const* );
	DEFINE_GETRECORDATTRIB( Sound, HRECORD );
	DEFINE_GETRECORDATTRIB( FadeInTime, float );	
	DEFINE_GETRECORDATTRIB( HoldTime, float );	
	DEFINE_GETRECORDATTRIB( FadeOutTime, float );	
	DEFINE_GETRECORDATTRIB( DelayTime, float );	
	DEFINE_GETRECORDATTRIB( FadeStyle, char const* );
	DEFINE_GETRECORDATTRIB( FlickerSound, HRECORD );
	DEFINE_GETRECORDSTRUCT( Lines );

	DEFINE_GETSTRUCTATTRIB( Lines, LineLayout, const char * );
	DEFINE_GETSTRUCTATTRIB( Lines, TextColor, uint32 );
	DEFINE_GETSTRUCTATTRIB( Lines, Line, const char * );

END_DATABASE_CATEGORY( );


BEGIN_DATABASE_CATEGORY( CreditOrder, "Interface/Credits/Order" )
	DEFINE_GETRECORDATTRIB( Entries, HRECORD );
END_DATABASE_CATEGORY( );

BEGIN_DATABASE_CATEGORY( CreditLineLayout, "Interface/Credits/LineLayout" )
	DEFINE_GETRECORDATTRIB( Font, HRECORD );
	DEFINE_GETRECORDATTRIB( TextSize, uint32 );
	DEFINE_GETRECORDATTRIB( BaseWidth, uint32 );
END_DATABASE_CATEGORY( );


#endif // __CREDITSDB_H__
