// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionAttackCrouch.h
//
// PURPOSE : AIActionAttackCrouch class definition
//
// CREATED : 3/18/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIACTION_ATTACK_CROUCH_H__
#define __AIACTION_ATTACK_CROUCH_H__

#include "AIActionAttack.h"


class CAIActionAttackCrouch : public CAIActionAttack
{
	typedef CAIActionAttack super;

	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionAttackCrouch, kAct_AttackCrouch );

		// CAIActionAbstract members.

		virtual bool	ValidateContextPreconditions( CAI* pAI, CAIWorldState& wsWorldStateGoal, bool bIsPlanning );
		virtual void	ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal );
		virtual bool	ValidateAction( CAI* pAI );

	protected:

		virtual void	SetAttackAnimProps( CAI* pAI, CAnimationProps* pProps );
};

// ----------------------------------------------------------------------- //

#endif
