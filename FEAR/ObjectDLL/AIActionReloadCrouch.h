// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionReloadCrouch.h
//
// PURPOSE : AIActionReloadCrouch class definition
//
// CREATED : 6/23/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIACTION_RELOAD_CROUCH_H__
#define __AIACTION_RELOAD_CROUCH_H__

#include "AIActionReload.h"


class CAIActionReloadCrouch : public CAIActionReload
{
	typedef CAIActionReload super;

	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionReloadCrouch, kAct_ReloadCrouch );

		CAIActionReloadCrouch();
};

// ----------------------------------------------------------------------- //

#endif
