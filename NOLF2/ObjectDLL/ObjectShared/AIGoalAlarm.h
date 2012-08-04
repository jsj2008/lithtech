// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalAlarm.h
//
// PURPOSE : AIGoalAlarm class definition
//
// CREATED : 10/26/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIGOAL_ALARM_H__
#define __AIGOAL_ALARM_H__

#include "AIGoalAbstractUseObject.h"


class CAIGoalAlarm : public CAIGoalAbstractUseObject
{
	typedef CAIGoalAbstractUseObject super;

	public:

		DECLARE_AI_FACTORY_CLASS_SPECIFIC(Goal, CAIGoalAlarm, kGoal_Alarm);

		CAIGoalAlarm( );

		// Save / Load

		virtual void	Save(ILTMessage_Write *pMsg);
        virtual void	Load(ILTMessage_Read *pMsg);

		// Activation.

		virtual void ActivateGoal();

		// Updating.

		virtual void UpdateGoal();

		// Attractor Handling.

		virtual AINode* FindNearestAttractorNode();

		// Triggered AI Sounds.

		virtual LTBOOL SelectTriggeredAISound(EnumAISoundType* peAISoundType);

		// Sense Handling.

		virtual LTBOOL HandleGoalSenseTrigger(AISenseRecord* pSenseRecord);

	protected:

		// State Handling.

		virtual void HandleStateUseObject();
		void HandleStateGoto();

	protected:

		LTVector	m_vEnemySeenPos;
};


#endif
