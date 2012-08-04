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

#include "stdafx.h"
#include "RotatingDoor.h"

LINKFROM_MODULE( RotatingDoor );

//
// Add props...
//

BEGIN_CLASS(RotatingDoor)

	// Set the AWM type

	AWM_SET_TYPE_ROTATING

END_CLASS_DEFAULT_FLAGS(RotatingDoor, Door, NULL, NULL, CF_WORLDMODEL)


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

