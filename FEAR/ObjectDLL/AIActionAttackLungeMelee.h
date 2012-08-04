// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionAttackLungeMelee.h
//
// PURPOSE : AIActionAttackLungeMelee class definition
//
// CREATED : 9/28/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIACTION_ATTACK_LUNGE_Melee_H__
#define __AIACTION_ATTACK_LUNGE_Melee_H__

#include "AIActionAttackLungeUncloaked.h"


class CAIActionAttackLungeMelee : public CAIActionAttackLungeUncloaked
{
	typedef CAIActionAttackLungeUncloaked super;

	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionAttackLungeMelee, kAct_AttackLungeMelee );

		// CAIActionAbstract members.

		virtual void			ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal );
		virtual void			DeactivateAction( CAI* pAI );
};

// ----------------------------------------------------------------------- //

#endif
