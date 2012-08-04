// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalDrawWeapon.h
//
// PURPOSE : AIGoalDrawWeapon class definition
//
// CREATED : 10/17/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIGOAL_DRAW_WEAPON_H__
#define __AIGOAL_DRAW_WEAPON_H__

#include "AIGoalAbstractStimulated.h"


class CAIGoalDrawWeapon : public CAIGoalAbstractStimulated
{
	typedef CAIGoalAbstractStimulated super;

	public:

		DECLARE_AI_FACTORY_CLASS_SPECIFIC(Goal, CAIGoalDrawWeapon, kGoal_DrawWeapon);

		// Save / Load

		virtual void	Save(ILTMessage_Write *pMsg);
        virtual void	Load(ILTMessage_Read *pMsg);

		// Activation.

		virtual void ActivateGoal();

		// Updating.

		void UpdateGoal();

		// Damage Handling.

		virtual LTBOOL HandleDamage(const DamageStruct& damage);

		// Sense Handling.

		virtual LTBOOL HandleGoalSenseTrigger(AISenseRecord* pSenseRecord);

	protected:

		// State Handling.

		void HandleStateDraw();
};


#endif
