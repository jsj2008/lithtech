// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalLoveKitty.h
//
// PURPOSE : AIGoalLoveKitty class definition
//
// CREATED : 2/14/02  (awww... on valentines day)
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIGOAL_LOVE_KITTY_H__
#define __AIGOAL_LOVE_KITTY_H__

#include "AIGoalAbstractStimulated.h"


class CAIGoalLoveKitty : public CAIGoalAbstractStimulated
{
	typedef CAIGoalAbstractStimulated super;

	public:

		DECLARE_AI_FACTORY_CLASS_SPECIFIC(Goal, CAIGoalLoveKitty, kGoal_LoveKitty);

		CAIGoalLoveKitty( );

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

		void HandleStateAnimate();
		void HandleStateGoto();

	protected:

		LTVector			m_vDest;
		LTBOOL				m_bIHateCats;
};


#endif
