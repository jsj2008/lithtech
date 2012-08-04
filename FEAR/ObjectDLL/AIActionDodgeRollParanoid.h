// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionDodgeRollParanoid.h
//
// PURPOSE : AIActionDodgeRollParanoid class definition
//
// CREATED : 4/28/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIACTION_DODGE_ROLL_PARANOID_H__
#define __AIACTION_DODGE_ROLL_PARANOID_H__

#include "AIActionDodgeRoll.h"


class CAIActionDodgeRollParanoid : public CAIActionDodgeRoll
{
	typedef CAIActionDodgeRoll super;

	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionDodgeRollParanoid, kAct_DodgeRollParanoid );

		// CAIActionAbstract members.

		virtual bool			ValidateContextPreconditions( CAI* pAI, CAIWorldState& wsWorldStateGoal, bool bIsPlanning );
		virtual void			ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal );
};

// ----------------------------------------------------------------------- //

#endif
