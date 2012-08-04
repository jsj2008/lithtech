// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalRetreat.h
//
// PURPOSE : AIGoalRetreat class definition
//
// CREATED : 10/16/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIGOAL_LUNGE_H__
#define __AIGOAL_LUNGE_H__

#include "AIGoalAbstractStimulated.h"


class CAIGoalRetreat : public CAIGoalAbstractStimulated
{
	typedef CAIGoalAbstractStimulated super;

	public:

		DECLARE_AI_FACTORY_CLASS_SPECIFIC(Goal, CAIGoalRetreat, kGoal_Retreat);

		CAIGoalRetreat( );

		// Save / Load

		virtual void	Save(ILTMessage_Write *pMsg);
        virtual void	Load(ILTMessage_Read *pMsg);

		// Initialization.

		virtual void InitGoal(CAI* pAI, LTFLOAT fImportance, LTFLOAT fTime);

		// Activation.

		virtual void ActivateGoal();
		virtual void DeactivateGoal();

		// Updating.

		void UpdateGoal();

		// Sense Handling.

		virtual LTBOOL HandleGoalSenseTrigger(AISenseRecord* pSenseRecord);

	protected:

		// State Handling.

		void HandleStateLongJump();

	protected:

		LTFLOAT		m_fRetreatTriggerDistSqr;
		LTFLOAT		m_fRetreatJumpDist;

		LTFLOAT		m_fRetreatSpeed;

		LTVector	m_vRetreatDest;
};


#endif
