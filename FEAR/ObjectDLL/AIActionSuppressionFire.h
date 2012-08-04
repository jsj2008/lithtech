// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionSuppressionFire.h
//
// PURPOSE : AIActionSuppressionFire class definition
//
// CREATED : 5/30/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIACTION_SUPPRESSION_FIRE_H__
#define __AIACTION_SUPPRESSION_FIRE_H__

#include "AIActionAttack.h"

#define SUPPRESSION_TIMEOUT		10.f


class CAIActionSuppressionFire : public CAIActionAttack
{
	typedef CAIActionAttack super;

	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionSuppressionFire, kAct_SuppressionFire );

		CAIActionSuppressionFire();

		// CAIActionAbstract members.

		virtual void	InitAction( AIDB_ActionRecord* pActionRecord );
		virtual bool	ValidateContextPreconditions( CAI* pAI, CAIWorldState& wsWorldStateGoal, bool bIsPlanning );
		virtual void	ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal );
		virtual void	DeactivateAction( CAI* pAI );
		virtual	bool	ValidateAction( CAI* pAI );

	protected:

		virtual void	SetAttackAnimProps( CAI* pAI, CAnimationProps* pProps );
};

// ----------------------------------------------------------------------- //

#endif
