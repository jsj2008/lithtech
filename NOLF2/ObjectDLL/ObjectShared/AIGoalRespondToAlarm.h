// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalRespondToAlarm.h
//
// PURPOSE : AIGoalRespondToAlarm class definition
//
// CREATED : 11/07/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIGOAL_RESPONDTOALARM_H__
#define __AIGOAL_RESPONDTOALARM_H__

#include "AIGoalAbstractSearch.h"

enum EnumAIStimulusID;

class CAIGoalRespondToAlarm : public CAIGoalAbstractSearch
{
	typedef CAIGoalAbstractSearch super;

	public:

		DECLARE_AI_FACTORY_CLASS_SPECIFIC(Goal, CAIGoalRespondToAlarm, kGoal_RespondToAlarm);

		CAIGoalRespondToAlarm( );

		// Save / Load

		virtual void	Save(ILTMessage_Write *pMsg);
        virtual void	Load(ILTMessage_Read *pMsg);

		// Activation.

		virtual void ActivateGoal();
		virtual void DeactivateGoal();

		// Volume Handling.

		virtual void HandleVolumeEnter(AIVolume* pVolume);
		virtual void HandleVolumeExit(AIVolume* pVolume);

		// Updating.

		void UpdateGoal();

		// Sense Handling.

		virtual LTBOOL HandleGoalSenseTrigger(AISenseRecord* pSenseRecord);

	protected:


		// State Setting.

		virtual void SetStateSearch();

		// State Handling.
		
		void HandleStateGoto();

	protected:

		LTVector			m_vDest;
		LTObjRef			m_hAlarm;
		EnumAIStimulusID	m_eStimIDAlarm;
		LTBOOL				m_bBuildSearchList;
		AIVolume*			m_pResponseVolume;
};


#endif
