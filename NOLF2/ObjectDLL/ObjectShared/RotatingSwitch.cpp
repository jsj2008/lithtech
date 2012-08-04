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

	#include "stdafx.h"
	#include "RotatingSwitch.h"

LINKFROM_MODULE( RotatingSwitch );

//
// Add props...
//

BEGIN_CLASS( RotatingSwitch )

	// Set the AWM type

	AWM_SET_TYPE_ROTATING	

END_CLASS_DEFAULT_FLAGS( RotatingSwitch, Switch, NULL, NULL, CF_WORLDMODEL )

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