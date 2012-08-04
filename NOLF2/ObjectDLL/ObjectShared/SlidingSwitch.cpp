// ----------------------------------------------------------------------- //
//
// MODULE  : SlidingSwitch.CPP
//
// PURPOSE : SlidingSwitch implementation
//
// CREATED : 5/29/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

//
// Includes...
//

#include "stdafx.h"
#include "SlidingSwitch.h"

LINKFROM_MODULE( SlidingSwitch );

//
// Add props...
//

BEGIN_CLASS( SlidingSwitch )

	// Set the AWM type

	AWM_SET_TYPE_SLIDING	

END_CLASS_DEFAULT_FLAGS( SlidingSwitch, Switch, NULL, NULL, CF_WORLDMODEL )

//
// Register the calss with the command mgr plugin and add any messages to the class
//

CMDMGR_BEGIN_REGISTER_CLASS( SlidingSwitch )
CMDMGR_END_REGISTER_CLASS( SlidingSwitch, Switch )


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	SlidingSwitch::RotatingSwitch
//
//  PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

SlidingSwitch::SlidingSwitch( )
:	Switch( )
{

}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	SlidingSwitch::~RotatingSwitch
//
//  PURPOSE:	Destroy object
//
// ----------------------------------------------------------------------- //

SlidingSwitch::~SlidingSwitch( )
{

}