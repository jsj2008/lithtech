// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenEffectDB.h
//
// PURPOSE : Defines an interface for accessing ScreenEffct data
//
// CREATED : 11/30/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __SCREENEFFECTDB_H__
#define __SCREENEFFECTDB_H__

//
// Includes...
//

#include "CategoryDB.h"

BEGIN_DATABASE_CATEGORY(ScreenEffect, "FX/ScreenEffect")
	DEFINE_GETRECORDATTRIB(FXName, char const*);
	DEFINE_GETRECORDATTRIB(Material, char const*);
	DEFINE_GETRECORDATTRIB(Parameters, char const*);
END_DATABASE_CATEGORY();

#endif // __SCREENEFFECTDB_H__
