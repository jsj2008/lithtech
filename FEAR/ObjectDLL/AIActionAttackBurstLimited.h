// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionAttackBurstLimited.h
//
// PURPOSE : AIActionAttackBurstLimited class definition
//
// CREATED : 4/06/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIACTION_ATTACK_BURST_LIMITED_H__
#define __AIACTION_ATTACK_BURST_LIMITED_H__

// Includes

#include "AIActionAttack.h"


class CAIActionAttackBurstLimited : public CAIActionAttack
{
	typedef CAIActionAttack super;

	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionAttackBurstLimited, kAct_AttackBurstLimited );

		// CAIActionAbstract members.

		virtual bool	ValidateContextPreconditions( CAI* pAI, CAIWorldState& wsWorldStateGoal, bool bIsPlanning );
		virtual	bool	ValidateAction( CAI* pAI );
};

// ----------------------------------------------------------------------- //

#endif
