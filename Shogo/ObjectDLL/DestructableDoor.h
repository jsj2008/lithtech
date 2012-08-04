// ----------------------------------------------------------------------- //
//
// MODULE  : DestructableDoor.h
//
// PURPOSE : A DestructableDoor object
//
// CREATED : 2/12/98
//
// ----------------------------------------------------------------------- //

#ifndef __DESTRUCTABLE_DOOR_H__
#define __DESTRUCTABLE_DOOR_H__

#include "DestructableWorldModel.h"
#include "door.h"

class DestructableDoor : public Door
{
	public:

		DestructableDoor();

	private:

		CDestructableWorldModel m_damage;
};


#endif // __DESTRUCTABLE_DOOR_H__