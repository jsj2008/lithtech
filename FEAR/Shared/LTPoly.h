// ----------------------------------------------------------------------- //
//
// MODULE  : LTPoly.h
//
// PURPOSE : Wrapper classes to initialize ltdrawprim structs
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __LTPOLY_H__
#define __LTPOLY_H__

//
// Includes...
//

	#include "iltdrawprim.h"

struct LTPoly_G4 : public LT_POLYG4
{
	LTPoly_G4()
	{
		memset( this, 0, sizeof(LTPoly_G4) );
	}
};

struct LTPoly_GT4 : public LT_POLYGT4
{
	LTPoly_GT4()
	{
		memset( this, 0, sizeof(LTPoly_GT4) );
	}
};

struct LTPoly_GTTS4 : public LT_POLYGTTS4
{
	LTPoly_GTTS4()
	{
		memset( this, 0, sizeof( LTPoly_GTTS4 ));
	}
};

#endif // __LTPOLY_H__