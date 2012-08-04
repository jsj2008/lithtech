// ------------------------------------------------------------------------- //
//
// FILE      : P S X 2 U N D A T A . C P P
//
// CREATED   : 11/01/99
//
// AUTHOR    : Matthew Scott
//
// COPYRIGHT : Monolith Productions Inc.
//
// ORIGN     : Loosly based upon struct banks and moArray's
//
// ------------------------------------------------------------------------- //

#include <stdio.h>
#include <stdlib.h>

#include "stdlterror.h"

#include "linuxundata.h"

// ------------------------------------------------------------------------- //
// Typedefs
// ------------------------------------------------------------------------- //
#define __UNDATA_NUMBLOCKS 2000

// ------------------------------------------------------------------------- //
// Externs (Public C data)
// ------------------------------------------------------------------------- //

// ------------------------------------------------------------------------- //
// Prototypes (Public C methods)
// ------------------------------------------------------------------------- //

// ------------------------------------------------------------------------- //
// Class Implementation
// ------------------------------------------------------------------------- //

// platform specific routine for seting up initial memory segment
ESTDLTResults 
CLINUXUnboundData::initUnboundHeap (uint32 blockSize, 
	uint32** begin, uint32** end) {
	if ((*begin = (uint32 *) malloc (blockSize*__UNDATA_NUMBLOCKS)) == NULL) {
		return (STDLT_ERROR);
	}
	*end   = *begin + ((blockSize*__UNDATA_NUMBLOCKS)/4);
	return (STDLT_OK);
}
