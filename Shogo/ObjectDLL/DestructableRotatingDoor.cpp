// ----------------------------------------------------------------------- //
//
// MODULE  : DestructableRotatingDoor.cpp
//
// PURPOSE : A DestructableRotatingDoor object
//
// CREATED : 6/28/98
//
// ----------------------------------------------------------------------- //

// Includes...
#include "DestructableRotatingDoor.h"

BEGIN_CLASS(DestructableRotatingDoor)
	ADD_DESTRUCTABLE_WORLD_MODEL_AGGREGATE()
END_CLASS_DEFAULT(DestructableRotatingDoor, RotatingDoor, NULL, NULL)


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DestructableRotatingDoor::DestructableRotatingDoor()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

DestructableRotatingDoor::DestructableRotatingDoor() : RotatingDoor()
{
	AddAggregate(&m_damage);
}
