// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionInstantDeathKnockedDown.h
//
// PURPOSE : AIActionInstantDeathKnockedDown class definition
//
// CREATED : 11/10/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIACTION_INSTANT_DEATH_KNOCKED_DOWN_H__
#define __AIACTION_INSTANT_DEATH_KNOCKED_DOWN_H__

#include "AIActionInstantDeath.h"


// Forward declarations.


class CAIActionInstantDeathKnockedDown : public CAIActionInstantDeath
{
	typedef CAIActionInstantDeath super;

	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionInstantDeathKnockedDown, kAct_InstantDeathKnockedDown );

		// CAIActionAbstract members.

		virtual bool	ValidateContextPreconditions( CAI* pAI, CAIWorldState& wsWorldStateGoal, bool bIsPlanning );
};

// ----------------------------------------------------------------------- //

#endif
