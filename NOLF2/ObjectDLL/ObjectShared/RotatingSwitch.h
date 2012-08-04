// ----------------------------------------------------------------------- //
//
// MODULE  : RotatingSwitch.h
//
// PURPOSE : A RotatingSwitch object
//
// CREATED : 5/29/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __ROTATING_SWITCH_H__
#define __ROTATING_SWITCH_H__

//
// Includes...
//

	#include "Switch.h"

LINKTO_MODULE( RotatingSwitch );

//
// Structs...
//

class RotatingSwitch : public Switch
{
	public: // Methods...

		RotatingSwitch( );
		virtual ~RotatingSwitch( );
};

#endif // __ROTATING_SWITCH_H__