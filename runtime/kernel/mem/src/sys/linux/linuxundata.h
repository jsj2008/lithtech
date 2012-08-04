// ------------------------------------------------------------------------- //
//
// FILE      : P S X 2 U N D A T A . H
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

#ifndef __LINUXUNDATA_H__
#define __LINUXUNDATA_H__

// ------------------------------------------------------------------------- //
// Includes this module depends on
// ------------------------------------------------------------------------- //
#include "stdlterror.h"

#include "undata.h"

// ------------------------------------------------------------------------- //
// Typedefs
// ------------------------------------------------------------------------- //

// ------------------------------------------------------------------------- //
// Externs (Public C data)
// ------------------------------------------------------------------------- //

// ------------------------------------------------------------------------- //
// Prototypes (Public C methods)
// ------------------------------------------------------------------------- //

// ------------------------------------------------------------------------- //
// Class Definitions
// ------------------------------------------------------------------------- //

class CLINUXUnboundData : public CGenUnboundData {

	// The rest of this class is defined and implemented in CGenUngoundData
	public:
		CLINUXUnboundData ();
		// platform specific routine for seting up initial memory segment
		ESTDLTResults 
		initUnboundHeap (uint32 blockSize, uint32** begin, uint32** end);

};

inline
CLINUXUnboundData::CLINUXUnboundData() {
	// this will call the setup method of our
	// parent class CGenUnboundData
	Init();
}

#endif  // __LINUXUNDATA_H__
