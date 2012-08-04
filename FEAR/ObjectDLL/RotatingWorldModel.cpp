// ----------------------------------------------------------------------- //
//
// MODULE  : RotatingWorldModel.cpp
//
// PURPOSE : RotatingWorldModel implementation
//
// CREATED : 10/27/97
//
// (c) 1997 - 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "RotatingWorldModel.h"

LINKFROM_MODULE( RotatingWorldModel );

BEGIN_CLASS( RotatingWorldModel )

	AWM_SET_TYPE_ROTATING	

END_CLASS_FLAGS(RotatingWorldModel, ActiveWorldModel, CF_WORLDMODEL, "A RotatingWorldModel is a WorldModel object that rotates around a specified point.")

//
// Register the calss with the command mgr plugin and add any messages to the class
//

CMDMGR_BEGIN_REGISTER_CLASS( RotatingWorldModel )
CMDMGR_END_REGISTER_CLASS( RotatingWorldModel, ActiveWorldModel )


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	RotatingWorldModel::RotatingWorldModel
//
//  PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

RotatingWorldModel::RotatingWorldModel( )
:	ActiveWorldModel( )
{

}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	RotatingWorldModel::~RotatingWorldModel
//
//  PURPOSE:	Destroy object
//
// ----------------------------------------------------------------------- //

RotatingWorldModel::~RotatingWorldModel( )
{

}
