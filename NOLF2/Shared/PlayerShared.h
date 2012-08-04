// ----------------------------------------------------------------------- //
//
// MODULE  : PlayerShared.h
//
// PURPOSE : Types and globals shared between the client and the server
//			 associated with the player.
//
// CREATED : 09/06/00
//
// (c) 2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __PLAYER_SHARED_H__
#define __PLAYER_SHARED_H__

#include "LTVector.h"
#include "ModelButeMgr.h"

// Shared player stuff...

const LTVector g_kvPlayerCameraOffset(0.0, 41.0, 0.0);
const LTVector g_kvPlayerScubaCameraOffset(0.0, 30.0, 0.0);

inline LTVector GetPlayerHeadOffset( ) 
{
	// Always use normal offsets in multiplayer...
	return g_kvPlayerCameraOffset; 
}

#endif // __PLAYER_SHARED_H__