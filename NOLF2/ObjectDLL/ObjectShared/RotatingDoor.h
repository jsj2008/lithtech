// ----------------------------------------------------------------------- //
//
// MODULE  : RotatingDoor.h
//
// PURPOSE : A RotatingDoor object
//
// CREATED : 12/3/97
//
// (c) 1997 - 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __ROTATING_DOOR_H__
#define __ROTATING_DOOR_H__

//
// Includes...
//

	#include "door.h"

LINKTO_MODULE( RotatingDoor );

//
// Structs...
//

class RotatingDoor : public Door
{
	public: // Methods...

		RotatingDoor();
		virtual ~RotatingDoor();

		LTBOOL IsStuck() const { return /*m_bStuck*/0; }

};


#endif // __ROTATING_DOOR_H__