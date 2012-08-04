// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionAttackReady.h
//
// PURPOSE : AIActionAttackReady class definition
//
// CREATED : 11/18/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIACTION_ATTACK_READY_H__
#define __AIACTION_ATTACK_READY_H__

#include "AIActionAttack.h"


class CAIActionAttackReady : public CAIActionAttack
{
	typedef CAIActionAttack super;

	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionAttackReady, kAct_AttackReady );

		CAIActionAttackReady();

		// CAIActionAbstract members.

		virtual bool	ValidateContextPreconditions( CAI* pAI, CAIWorldState& wsWorldStateGoal, bool bIsPlanning );
		virtual void	ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal );
		virtual bool	ValidateAction( CAI* pAI );

	protected:

		virtual void	SetAttackAnimProps( CAI* pAI, CAnimationProps* pProps );
};

// ----------------------------------------------------------------------- //

#endif
