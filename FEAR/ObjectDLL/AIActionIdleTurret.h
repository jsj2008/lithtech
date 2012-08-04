// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionIdleTurret.h
//
// PURPOSE : AIActionIdleTurret class definition
//
// CREATED : 6/29/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIACTION_IDLE_TURRET_H__
#define __AIACTION_IDLE_TURRET_H__

#include "AIActionIdle.h"


class CAIActionIdleTurret : public CAIActionIdle
{
	typedef CAIActionIdle super;

	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionIdleTurret, kAct_IdleTurret );


		// CAIActionAbstract members.

		virtual void	ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal );
		virtual	bool	ValidateAction( CAI* pAI );
};

// ----------------------------------------------------------------------- //

#endif
