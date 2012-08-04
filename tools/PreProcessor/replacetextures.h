
// This module replaces textures in a CPreWorld with the ones in a CEditRegion.
// It does this by finding the closest surface to each CPreWorld surface.

#ifndef __REPLACETEXTURES_H__
#define __REPLACETEXTURES_H__


	#include "preworld.h"

	
	int ReplaceTextures(CEditRegion *pRegion, CPreMainWorld *pWorld);


#endif  // __REPLACETEXTURES_H__




