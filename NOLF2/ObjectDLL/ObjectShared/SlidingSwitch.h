// ----------------------------------------------------------------------- //
//
// MODULE  : SlidingSwitch.h
//
// PURPOSE : A SlidingSwitch object
//
// CREATED : 5/29/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __SLIDING_SWITCH_H__
#define __SLIDING_SWITCH_H__

//
// Includes...
//

	#include "Switch.h"

LINKTO_MODULE( SlidingSwitch );

//
// Structs...
//

class SlidingSwitch : public Switch
{
	public: // Methods...

		SlidingSwitch( );
		virtual ~SlidingSwitch( );
};

#endif // __SLIDING_SWITCH_H__