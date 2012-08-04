// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionReloadCovered.h
//
// PURPOSE : AIActionReloadCovered class definition
//
// CREATED : 5/16/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIACTION_RELOAD_COVERED_H__
#define __AIACTION_RELOAD_COVERED_H__

#include "AIActionReload.h"


class CAIActionReloadCovered : public CAIActionReload
{
	typedef CAIActionReload super;

	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionReloadCovered, kAct_ReloadCovered );

		CAIActionReloadCovered();

		// CAIActionAbstract members.

		virtual bool	ValidateContextPreconditions( CAI* pAI, CAIWorldState& wsWorldStateGoal, bool bIsPlanning );
		virtual void	ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal );
};

// ----------------------------------------------------------------------- //

#endif
