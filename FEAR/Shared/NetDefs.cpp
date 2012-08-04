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

#include "Stdafx.h"
#include "NetDefs.h"

// Guids...

const BandWidthDefaults g_BandwidthClient[eBandwidth_Custom] =
{
//	{ 35, 15 },		// eBandwidth_56K
	{ 256, 20 },	// eBandwidth_DSL_Low
	{ 512, 25 },	// eBandwidth_DSL_High
	{ 1024, 30 },	// eBandwidth_Cable
	{ 1500, 30 },	// eBandwidth_T1
	{ 10000, 30 },	// eBandwidth_LAN
};

const uint16 g_BandwidthServer[eBandwidth_Custom] =
{
//	30,			// eBandwidth_56K
	128,		// eBandwidth_DSL_Low
	256,		// eBandwidth_DSL_High
	256,		// eBandwidth_Cable
	1500,		// eBandwidth_T1
	10000		// eBandwidth_LAN
};

const BandwidthMaxPlayers g_BandwidthMaxPlayers[] = 
{
	{ 0, 1 },
	{ 32, 2 },
	{ 80, 3 },
	{ 128, 4 },
	{ 192, 5 },
	{ 256, 6 },
	{ 384, 7 },
	{ 512, 8 },
	{ 768, 12 },
	{ 1500, 16 },
};
const uint32 g_BandwidthMaxPlayersSize = LTARRAYSIZE( g_BandwidthMaxPlayers );

