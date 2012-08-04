// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalAttackRanged.h
//
// PURPOSE : AIGoalAttackRanged class definition
//
// CREATED : 10/09/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIGOAL_ATTACK_RANGED_H__
#define __AIGOAL_ATTACK_RANGED_H__

#include "AIGoalAttack.h"


class CAIGoalAttackRanged : public CAIGoalAttack
{
	typedef CAIGoalAttack super;

	public:

		DECLARE_AI_FACTORY_CLASS_SPECIFIC(Goal, CAIGoalAttackRanged, kGoal_AttackRanged);

		CAIGoalAttackRanged( );

		// Save / Load

		virtual void	Save(ILTMessage_Write *pMsg);
        virtual void	Load(ILTMessage_Read *pMsg);

		// Activation.

		virtual void ActivateGoal();

	protected:

};


#endif
