// ----------------------------------------------------------------------- //
//
// MODULE  : SlidingDoor.cpp
//
// PURPOSE : SlidingDoor implementation
//
// CREATED : 5/21/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

//
// Includes...
//

#include "stdafx.h"
#include "SlidingDoor.h"

LINKFROM_MODULE( SlidingDoor );


//
// Add props...
//

BEGIN_CLASS( SlidingDoor )

	// Set the type

	AWM_SET_TYPE_SLIDING

	// Overrides...

	ADD_BOOLPROP_FLAG(OpenAway, LTFALSE, PF_GROUP(3) | PF_HIDDEN) 

END_CLASS_DEFAULT_FLAGS( SlidingDoor, Door, NULL, NULL, CF_WORLDMODEL)

//
// Register the calss with the command mgr plugin and add any messages to the class
//

CMDMGR_BEGIN_REGISTER_CLASS( SlidingDoor )
CMDMGR_END_REGISTER_CLASS( SlidingDoor, Door )

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	SlidingDoor::SlidingDoor
//
//  PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

SlidingDoor::SlidingDoor( )
:	Door( )
{

}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	SlidingDoor::~SlidingDoor
//
//  PURPOSE:	Destroy object
//
// ----------------------------------------------------------------------- //

SlidingDoor::~SlidingDoor( )
{

}

