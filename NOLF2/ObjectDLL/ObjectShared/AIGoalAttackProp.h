// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalAttackProp.h
//
// PURPOSE : AIGoalAttackProp class definition
//
// CREATED : 7/25/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIGOAL_ATTACK_PROP_H__
#define __AIGOAL_ATTACK_PROP_H__

#include "AIGoalAttack.h"


class CAIGoalAttackProp : public CAIGoalAttack
{
	typedef CAIGoalAttack super;

	public:

		DECLARE_AI_FACTORY_CLASS_SPECIFIC(Goal, CAIGoalAttackProp, kGoal_AttackProp);

		// Save / Load

		virtual void	Save(ILTMessage_Write *pMsg);
        virtual void	Load(ILTMessage_Read *pMsg);

		// Activation.

		virtual void ActivateGoal();

		// Updating.

		void UpdateGoal();

		// Attractor Handling.

		virtual AINode*	HandleGoalAttractors();

	protected:

		// State Setting.

		virtual void SetStateAttack();

		// State Handling.

		void HandleStateAttackProp();
		void HandleStateDraw();

	protected:

};


#endif
