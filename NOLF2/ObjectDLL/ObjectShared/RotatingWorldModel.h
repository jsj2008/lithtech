// ----------------------------------------------------------------------- //
//
// MODULE  : RotatingWorldModel.h
//
// PURPOSE : RotatingWorldModel definition
//
// CREATED : 10/27/97
//
// (c) 1997 - 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __ROTATING_WORLD_MODEL_H__
#define __ROTATING_WORLD_MODEL_H__

#include "ActiveWorldModel.h"

LINKTO_MODULE( RotatingWorldModel );


class RotatingWorldModel : public ActiveWorldModel
{
	public: // Methods...

		RotatingWorldModel( );
		virtual ~RotatingWorldModel( );

};

#endif // __ROTATING_WORLD_MODEL_H__