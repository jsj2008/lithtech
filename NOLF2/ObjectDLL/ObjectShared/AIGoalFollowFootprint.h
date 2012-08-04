// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalFollowFootprint.h
//
// PURPOSE : AIGoalFollowFootprint class definition
//
// CREATED : 7/26/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIGOAL_FOLLOW_FOOTPRINT_H__
#define __AIGOAL_FOLLOW_FOOTPRINT_H__

#include "AIGoalAbstractSearch.h"


class CAIGoalFollowFootprint : public CAIGoalAbstractSearch
{
	typedef CAIGoalAbstractSearch super;

	public:

		DECLARE_AI_FACTORY_CLASS_SPECIFIC(Goal, CAIGoalFollowFootprint, kGoal_FollowFootprint);

		CAIGoalFollowFootprint( );

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

		void HandleStateFollowFootprint();

	protected:

		LTVector			m_vStimulusPos;
		LTFLOAT				m_fStimulationTime;
};


#endif
