// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalAttackMelee.h
//
// PURPOSE : AIGoalAttackMelee class definition
//
// CREATED : 10/09/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIGOAL_ATTACK_MELEE_H__
#define __AIGOAL_ATTACK_MELEE_H__

#include "AIGoalAttack.h"


class CAIGoalAttackMelee : public CAIGoalAttack
{
	typedef CAIGoalAttack super;

	public:

		DECLARE_AI_FACTORY_CLASS_SPECIFIC(Goal, CAIGoalAttackMelee, kGoal_AttackMelee);

		CAIGoalAttackMelee( );

		// Save / Load

		virtual void	Save(ILTMessage_Write *pMsg);
        virtual void	Load(ILTMessage_Read *pMsg);

		// Activation.

		virtual void ActivateGoal();

	protected:

};


#endif
