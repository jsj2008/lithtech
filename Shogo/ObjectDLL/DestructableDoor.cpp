// ----------------------------------------------------------------------- //
//
// MODULE  : DestructableDoor.cpp
//
// PURPOSE : A DestructableDoor object
//
// CREATED : 2/12/98
//
// ----------------------------------------------------------------------- //

// Includes...
#include "DestructableDoor.h"

BEGIN_CLASS(DestructableDoor)
	ADD_DESTRUCTABLE_WORLD_MODEL_AGGREGATE()
END_CLASS_DEFAULT(DestructableDoor, Door, NULL, NULL)


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DestructableDoor::DestructableDoor()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

DestructableDoor::DestructableDoor() : Door()
{
	AddAggregate(&m_damage);
}
