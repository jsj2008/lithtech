/****************************************************************************
;
;	 MODULE:		NetDefs (.CPP)
;
;	PURPOSE:		Network game definitions
;
;	HISTORY:		11/12/99 [kls] This file was created
;
;	COMMENT:		Copyright (c) 1999, Monolith Productions Inc.
;
****************************************************************************/

#include "stdafx.h"
#include "NetDefs.h"

// Guids...

static const char* g_kaGameTypeString[] =
{
	"Single",
	"Cooperative",
	"Deathmatch",
	"TeamDeathmatch",
	"DoomsDay"
};

const uint16 g_BandwidthClient[eBandwidth_Custom] =
{
	35,
	256,
	512,
	1024,
	1500,
	10000
};

const uint16 g_BandwidthServer[eBandwidth_Custom] =
{
	30,
	128,
	256,
	256,
	1500,
	10000
};

const char* GameTypeToString(GameType eType)
{
	if (eType >= 0 && eType < g_knNumGameTypes)
	{
		return g_kaGameTypeString[eType];
	}

    return NULL;
}

GameType GameStringTypeToGameType( char const* pszGameType )
{
	for( int i = 0; i < g_knNumGameTypes; i++ )
	{
		if( strcmp( pszGameType, g_kaGameTypeString[i] ) == 0 )
			return ( GameType )i;
	}

	return eGameTypeSingle;
}