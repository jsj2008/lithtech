// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionAttackMeleeUncloaked.h
//
// PURPOSE : AIActionAttackMeleeUncloaked class definition
//
// CREATED : 4/24/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIACTION_ATTACK_MELEE_UNCLOAKED_H__
#define __AIACTION_ATTACK_MELEE_UNCLOAKED_H__

#include "AIActionAttackMelee.h"

class CAIActionAttackMeleeUncloaked : public CAIActionAttackMelee
{
	typedef CAIActionAttackMelee super;

	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionAttackMeleeUncloaked, kAct_AttackMeleeUncloaked );

		// CAIActionAbstract members.

		virtual void			ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal );
		virtual void			DeactivateAction( CAI* pAI );
};

#endif
