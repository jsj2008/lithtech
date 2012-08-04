// ----------------------------------------------------------------------- //
//
// MODULE  : SlidingWorldModel.h
//
// PURPOSE : The SlidingWorldModel object
//
// CREATED : 5/21/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef	__SLIDING_WORLD_MODEL_H__
#define __SLIDING_WORLD_MODEL_H__

//
// Includes...
//

	#include "ActiveWorldModel.h"

LINKTO_MODULE( SlidingWorldModel );

//
// Structs...
//

class SlidingWorldModel : public ActiveWorldModel
{
	public: // Methods...

		SlidingWorldModel( );
		virtual ~SlidingWorldModel();

	private:

		PREVENT_OBJECT_COPYING( SlidingWorldModel );
};

#endif // __SLIDING_WORLD_MODEL_H__
