// ----------------------------------------------------------------------- //
//
// MODULE  : RotatingDoor.CPP
//
// PURPOSE : RotatingDoor implementation
//
// CREATED : 12/3/97
//
// (c) 1997 - 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

//
// Includes...
// 

#include "Stdafx.h"
#include "RotatingDoor.h"

LINKFROM_MODULE( RotatingDoor );

//
// Add props...
//

BEGIN_CLASS(RotatingDoor)

	// Set the AWM type

	AWM_SET_TYPE_ROTATING

	ADD_PREFETCH_RESOURCE_PROPS()

END_CLASS_FLAGS_PREFETCH(RotatingDoor, Door, CF_WORLDMODEL, DefaultPrefetch<Door>, "A RotatingDoor is a Door object that rotates around a specified point.")



//
// Register the calss with the command mgr plugin and add any messages to the class
//

CMDMGR_BEGIN_REGISTER_CLASS( RotatingDoor )
CMDMGR_END_REGISTER_CLASS( RotatingDoor, Door )


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	RotatingDoor::RotatingDoor
//
//  PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

RotatingDoor::RotatingDoor( )
:	Door()
{

}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	RotatingDoor::~RotatingDoor
//
//  PURPOSE:	Destroy object
//
// ----------------------------------------------------------------------- //

RotatingDoor::~RotatingDoor( )
{

}

