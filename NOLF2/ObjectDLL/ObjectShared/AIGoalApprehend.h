// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalApprehend.h
//
// PURPOSE : AIGoalApprehend class definition
//
// CREATED : 1/25/02
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIGOAL_APPREHEND_H__
#define __AIGOAL_APPREHEND_H__

#include "AIGoalAbstractStimulated.h"


class CAIGoalApprehend : public CAIGoalAbstractStimulated
{
	typedef CAIGoalAbstractStimulated super;

	public:

		DECLARE_AI_FACTORY_CLASS_SPECIFIC(Goal, CAIGoalApprehend, kGoal_Apprehend);

		CAIGoalApprehend( );

		// Save / Load

		virtual void	Save(ILTMessage_Write *pMsg);
        virtual void	Load(ILTMessage_Read *pMsg);

		// Activation.

		virtual void ActivateGoal();

		// Updating.

		void UpdateGoal();

		// Sense Handling.

		virtual LTBOOL HandleGoalSenseTrigger(AISenseRecord* pSenseRecord);

	protected:

		// State Handling.

		void HandleStateApprehend();

	protected:

		LTFLOAT		m_fHoldTime;
};


#endif
