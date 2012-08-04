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

	#include "Door.h"

LINKTO_MODULE( RotatingDoor );

//
// Structs...
//

class RotatingDoor : public Door
{
	public: // Methods...

		RotatingDoor();
		virtual ~RotatingDoor();

		bool IsStuck() const { return /*m_bStuck*/0; }

	private:

		PREVENT_OBJECT_COPYING( RotatingDoor );

};


#endif // __ROTATING_DOOR_H__
