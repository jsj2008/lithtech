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

struct LTPoly_GT4 : public LT_POLYGT4
{
	LTPoly_GT4()
	{
		memset( this, 0, sizeof(LTPoly_GT4) );
	}
};

#endif // __LTPOLY_H__