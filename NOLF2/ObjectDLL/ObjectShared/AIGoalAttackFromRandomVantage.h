// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalAttackFromRandomVantage.h
//
// PURPOSE : AIGoalAttackFromRandomVantage class definition
//
// CREATED : 2/20/02
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIGOAL_ATTACK_FROM_RANDOM_VANTAGE_H__
#define __AIGOAL_ATTACK_FROM_RANDOM_VANTAGE_H__

#include "AIGoalAttackFromVantage.h"


class CAIGoalAttackFromRandomVantage : public CAIGoalAttackFromVantage
{
	typedef CAIGoalAttackFromVantage super;

	public:

		DECLARE_AI_FACTORY_CLASS_SPECIFIC(Goal, CAIGoalAttackFromRandomVantage, kGoal_AttackFromRandomVantage);

		// Attractor Handling.

		virtual AINode* FindNearestAttractorNode(EnumAINodeType eNodeType, const LTVector& vPos, LTFLOAT fDistSqr);

	protected:

		// State Handling.

		virtual void SetStateAttack();
		virtual void HandleStateAttackFromVantage();
};


#endif
