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

#include "Stdafx.h"
#include "SlidingSwitch.h"

LINKFROM_MODULE( SlidingSwitch );

//
// Add props...
//

BEGIN_CLASS( SlidingSwitch )

	// Set the AWM type

	AWM_SET_TYPE_SLIDING	

	ADD_PREFETCH_RESOURCE_PROPS()

END_CLASS_FLAGS_PREFETCH(SlidingSwitch, Switch, CF_WORLDMODEL, DefaultPrefetch<Switch>, "A SlidingSwitch is a Switch object that moves a specified distance in a specified direction.")


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
