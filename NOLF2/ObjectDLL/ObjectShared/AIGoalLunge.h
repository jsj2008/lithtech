// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalLunge.h
//
// PURPOSE : AIGoalLunge class definition
//
// CREATED : 10/10/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIGOAL_LUNGE_H__
#define __AIGOAL_LUNGE_H__

#include "AIGoalAbstractStimulated.h"


class CAIGoalLunge : public CAIGoalAbstractStimulated
{
	typedef CAIGoalAbstractStimulated super;

	public:

		DECLARE_AI_FACTORY_CLASS_SPECIFIC(Goal, CAIGoalLunge, kGoal_Lunge);

		CAIGoalLunge( );

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

		LTBOOL Retreat();

		// State Handling.

		void HandleStateLongJump();

	protected:

		LTVector	m_vOrigPos;
		LTBOOL		m_bLunged;

		LTFLOAT		m_fLungeDistSqrMin;
		LTFLOAT		m_fLungeDistSqrMax;

		LTFLOAT		m_fLungeSpeed;

		LTVector	m_vLungeDest;
};


#endif
