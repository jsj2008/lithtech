// ----------------------------------------------------------------------- //
//
// MODULE  : Breakable.CPP
//
// PURPOSE : A Breakable object
//
// CREATED : 1/14/99
//
// (c) 1999-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "Breakable.h"

LINKFROM_MODULE( Breakable );

BEGIN_CLASS(Breakable)

	// Overrides...

	ADD_BOOLPROP_FLAG(NeverDestroy, LTFALSE, PF_GROUP(1))
	ADD_REALPROP_FLAG(Armor, 0.0f, PF_GROUP(1))


END_CLASS_DEFAULT_FLAGS(Breakable, ActiveWorldModel, NULL, NULL, CF_WORLDMODEL)


CMDMGR_BEGIN_REGISTER_CLASS( Breakable )
CMDMGR_END_REGISTER_CLASS( Breakable, ActiveWorldModel )
