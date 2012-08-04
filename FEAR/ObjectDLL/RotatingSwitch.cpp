// ----------------------------------------------------------------------- //
//
// MODULE  : RotatingSwitch.CPP
//
// PURPOSE : RotatingSwitch implementation
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
	#include "RotatingSwitch.h"

LINKFROM_MODULE( RotatingSwitch );

//
// Add props...
//

BEGIN_CLASS( RotatingSwitch )

	// Set the AWM type

	AWM_SET_TYPE_ROTATING	

	ADD_PREFETCH_RESOURCE_PROPS()

END_CLASS_FLAGS_PREFETCH(RotatingSwitch, Switch, CF_WORLDMODEL, DefaultPrefetch<Switch>, "A RotatingSwitch is a Switch object that rotates around a specified point.")

//
// Register the calss with the command mgr plugin and add any messages to the class
//

CMDMGR_BEGIN_REGISTER_CLASS( RotatingSwitch )
CMDMGR_END_REGISTER_CLASS( RotatingSwitch, Switch )


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	RotatingSwitch::RotatingSwitch
//
//  PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

RotatingSwitch::RotatingSwitch( )
:	Switch( )
{

}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	RotatingSwitch::~RotatingSwitch
//
//  PURPOSE:	Destroy object
//
// ----------------------------------------------------------------------- //

RotatingSwitch::~RotatingSwitch( )
{

}
