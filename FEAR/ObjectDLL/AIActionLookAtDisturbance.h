// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionLookAtDisturbance.h
//
// PURPOSE : AIActionLookAtDisturbance class definition
//
// CREATED : 3/28/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIACTION_LOOK_AT_DISTURBANCE_H__
#define __AIACTION_LOOK_AT_DISTURBANCE_H__

#include "AIWorldState.h"
#include "AIActionAbstract.h"

// Forward declarations.

class	CAnimationProps;


class CAIActionLookAtDisturbance : public CAIActionAbstract
{
	typedef CAIActionAbstract super;

	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionLookAtDisturbance, kAct_LookAtDisturbance );

		CAIActionLookAtDisturbance();

		// CAIActionAbstract members.

		virtual void	InitAction( AIDB_ActionRecord* pActionRecord );
		virtual bool	ValidateContextPreconditions( CAI* pAI, CAIWorldState& wsWorldStateGoal, bool bIsPlanning );
		virtual void	ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal );
		virtual	bool	IsActionComplete( CAI* pAI );
		virtual void	ApplyContextEffect( CAI* pAI, CAIWorldState* pwsWorldStateCur, CAIWorldState* pwsWorldStateGoal );
};

// ----------------------------------------------------------------------- //

#endif
