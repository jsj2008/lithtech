// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalDisappearReappear.h
//
// PURPOSE : AIGoalDisappearReappear class definition
//
// CREATED : 11/09/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIGOAL_DISAPPEAR_REAPPEAR_H__
#define __AIGOAL_DISAPPEAR_REAPPEAR_H__

#include "AIGoalAbstractStimulated.h"


class CAIGoalDisappearReappear : public CAIGoalAbstractStimulated
{
	typedef CAIGoalAbstractStimulated super;

	public:

		DECLARE_AI_FACTORY_CLASS_SPECIFIC(Goal, CAIGoalDisappearReappear, kGoal_DisappearReappear);

		CAIGoalDisappearReappear( );

		// Save / Load

		virtual void	Save(ILTMessage_Write *pMsg);
        virtual void	Load(ILTMessage_Read *pMsg);

		// Initialization.

		virtual void	InitGoal(CAI* pAI);

		// Activation.

		virtual void ActivateGoal();
		virtual void DeactivateGoal();

		// Updating.

		void UpdateGoal();

		// Damage Handling.

		virtual LTBOOL HandleDamage(const DamageStruct& damage);

		// Sense Handling.

		virtual LTBOOL HandleGoalSenseTrigger(AISenseRecord* pSenseRecord);

	protected:

		// State Handling.

		virtual void HandleStateDisappearReappear();

	protected:

		LTFLOAT		m_fDisappearDistMinSqr;
		LTFLOAT		m_fDisappearDistMaxSqr;
};


#endif
