// ----------------------------------------------------------------------- //
//
// MODULE  : DestructableRotatingDoor.h
//
// PURPOSE : A DestructableRotatingDoor object
//
// CREATED : 6/28/98
//
// ----------------------------------------------------------------------- //

#ifndef __DESTRUCTABLE_ROTATING_DOOR_H__
#define __DESTRUCTABLE_ROTATING_DOOR_H__

#include "DestructableWorldModel.h"
#include "RotatingDoor.h"

class DestructableRotatingDoor : public RotatingDoor
{
	public:

		DestructableRotatingDoor();

	private:

		CDestructableWorldModel m_damage;
};


#endif // __DESTRUCTABLE_ROTATING_DOOR_H__