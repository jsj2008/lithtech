// ----------------------------------------------------------------------- //
//
// MODULE  : SlidingWorldModel.cpp
//
// PURPOSE : SlidingWorldModel implementation
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
	#include "SlidingWorldModel.h"

LINKFROM_MODULE( SlidingWorldModel );


//
// Add props...
//

BEGIN_CLASS( SlidingWorldModel )

	// Set the type

	AWM_SET_TYPE_SLIDING

END_CLASS_DEFAULT_FLAGS( SlidingWorldModel, ActiveWorldModel, NULL, NULL, CF_WORLDMODEL)

//
// Register the calss with the command mgr plugin and add any messages to the class
//

CMDMGR_BEGIN_REGISTER_CLASS( SlidingWorldModel )
CMDMGR_END_REGISTER_CLASS( SlidingWorldModel, ActiveWorldModel )


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	SlidingWorldModel::SlidingWorldModel
//
//  PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

SlidingWorldModel::SlidingWorldModel( )
:	ActiveWorldModel()
{
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	SlidingWorldModel::~SlidingWorldModel
//
//  PURPOSE:	Destroy object
//
// ----------------------------------------------------------------------- //

SlidingWorldModel::~SlidingWorldModel( )
{

}