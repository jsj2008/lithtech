// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalHolsterWeapon.h
//
// PURPOSE : AIGoalHolsterWeapon class definition
//
// CREATED : 11/02/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIGOAL_HOLSTER_WEAPON_H__
#define __AIGOAL_HOLSTER_WEAPON_H__

#include "AIGoalAbstract.h"


class CAIGoalHolsterWeapon : public CAIGoalAbstract
{
	typedef CAIGoalAbstract super;

	public:

		DECLARE_AI_FACTORY_CLASS_SPECIFIC(Goal, CAIGoalHolsterWeapon, kGoal_HolsterWeapon);

		// Save / Load

		virtual void	Save(ILTMessage_Write *pMsg);
        virtual void	Load(ILTMessage_Read *pMsg);

		// Activation.

		virtual void ActivateGoal();

		// Updating.

		void UpdateGoal();
		virtual void RecalcImportance();

	protected:

		// State Handling.

		void HandleStateHolster();
};


#endif
