// ----------------------------------------------------------------------- //
//
// MODULE  : SharedMovement.cpp
//
// PURPOSE : Shared movement implementation.
//
// CREATED : 8/27/99
//
// (c)	1999-2003 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "SharedMovement.h"

char* aPPMStrings[] =
{
	"Normal",
	"Vehicle",
	"Lure",
};

char* GetPropertyNameFromPlayerPhysicsModel(PlayerPhysicsModel ePPModel)
{
	if (PPM_FIRST <= ePPModel && ePPModel < PPM_NUM_MODELS)
	{
		return aPPMStrings[ePPModel];
	}

    return NULL;
}


PlayerPhysicsModel GetPlayerPhysicsModelFromPropertyName( const char* pName )
{ 
	if( !pName )
		return PPM_NORMAL;

	for( uint8 nPPM = 0; nPPM < PPM_NUM_MODELS; ++nPPM )
	{
		if( LTStrIEquals( aPPMStrings[nPPM], pName ))
		{
			return (PlayerPhysicsModel)nPPM;
		}
	}

	return PPM_NORMAL;
}

