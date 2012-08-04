// ----------------------------------------------------------------------- //
//
// MODULE  : Point.cpp
//
// PURPOSE : Point class implementation
//
// CREATED : 5/2/01
//
// ----------------------------------------------------------------------- //

//
//	Includes...
//

	#include "Stdafx.h"		
	#include "Point.h"


LINKFROM_MODULE( Point );

BEGIN_CLASS( Point )
END_CLASS( Point, BaseClass, "Point objects are used solely as position markers in levels.  Other game objects and systems refer to point objects only for their transfom information." )

CMDMGR_BEGIN_REGISTER_CLASS( Point )
CMDMGR_END_REGISTER_CLASS( Point, BaseClass )

