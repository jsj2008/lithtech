// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionInstantDeath.h
//
// PURPOSE : AIActionInstantDeath class definition
//
// CREATED : 03/29/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIACTION_INSTANT_DEATH_H__
#define __AIACTION_INSTANT_DEATH_H__

#include "AIActionAbstract.h"


// Forward declarations.


class CAIActionInstantDeath : public CAIActionAbstract
{
	typedef CAIActionAbstract super;

	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionInstantDeath, kAct_InstantDeath );

		CAIActionInstantDeath();

		// CAIActionAbstract members.

		virtual void	InitAction( AIDB_ActionRecord* pActionRecord );
		virtual void	ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal );
		virtual bool	ValidateContextPreconditions( CAI* pAI, CAIWorldState& wsWorldStateGoal, bool bIsPlanning );
};

// ----------------------------------------------------------------------- //

#endif
