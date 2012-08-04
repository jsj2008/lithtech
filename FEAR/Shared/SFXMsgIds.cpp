// ----------------------------------------------------------------------- //
//
// MODULE  : SFXMsgIds.cpp
//
// PURPOSE : This fine defines functions used for mapping strings to ids.
//
// CREATED : 3/29/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "SFXMsgIds.h"

//
// BodyState values..
//
static const char* g_pszBodyState[] =
{
#define BODYSTATE_AS_STRING 1
#include "BodyStateEnums.h"
#undef BODYSTATE_AS_STRING
};

BodyState StringToBodyState( const char* pszBodyState )
{
	if ( NULL == pszBodyState )
	{
		return eBodyStateInvalid;
	}

	for ( int i = 0; i < LTARRAYSIZE( g_pszBodyState ); ++i )
	{
		if ( LTStrIEquals( pszBodyState,  g_pszBodyState[i] ) )
		{
			return (BodyState)i;
		}
	}

	return eBodyStateInvalid;
}

