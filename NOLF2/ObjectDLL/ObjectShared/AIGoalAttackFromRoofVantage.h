// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalAttackFromRoofVantage.h
//
// PURPOSE : AIGoalAttackFromRoofVantage class definition
//
// CREATED : 5/18/02
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIGOAL_ATTACK_FROM_ROOF_VANTAGE_H__
#define __AIGOAL_ATTACK_FROM_ROOF_VANTAGE_H__

#include "AIGoalAttackFromVantage.h"


class CAIGoalAttackFromRoofVantage : public CAIGoalAttackFromVantage
{
	typedef CAIGoalAttackFromVantage super;

	public:

		DECLARE_AI_FACTORY_CLASS_SPECIFIC(Goal, CAIGoalAttackFromRoofVantage, kGoal_AttackFromRoofVantage);

		// Activation.

		virtual void DeactivateGoal();

		// Attractor Handling.

		virtual AINode* FindNearestAttractorNode(EnumAINodeType eNodeType, const LTVector& vPos, LTFLOAT fDistSqr);

	protected:

		virtual void SetStateAttack();
};


#endif
