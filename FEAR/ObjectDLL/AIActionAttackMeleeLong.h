// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionAttackMeleeLong.h
//
// PURPOSE : AIActionAttackMeleeLong class definition
//
// CREATED : 10/04/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIACTION_ATTACK_MELEE_LONG_H__
#define __AIACTION_ATTACK_MELEE_LONG_H__

#include "AIActionAttackLungeUncloaked.h"


class CAIActionAttackMeleeLong : public CAIActionAttackLungeUncloaked
{
	typedef CAIActionAttackLungeUncloaked super;

	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionAttackMeleeLong, kAct_AttackMeleeLong );

		// CAIActionAbstract members.

		virtual void			ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal );
		virtual void			DeactivateAction( CAI* pAI );
};

// ----------------------------------------------------------------------- //

#endif
