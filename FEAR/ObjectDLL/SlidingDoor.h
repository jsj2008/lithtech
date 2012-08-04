// ----------------------------------------------------------------------- //
//
// MODULE  : SlidingDoor.h
//
// PURPOSE : The SlidingDoor object
//
// CREATED : 5/21/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef	__SLIDING_DOOR_H__
#define __SLIDING_DOOR_H__

//
// Includes...
//

#include "Door.h"

LINKTO_MODULE( SlidingDoor );

//
// Structs...
//

class SlidingDoor : public Door
{
	public: // Methods...

		SlidingDoor( );
		virtual ~SlidingDoor( );

	private:

		PREVENT_OBJECT_COPYING( SlidingDoor );
};

#endif // __SLIDING_DOOR_H__
