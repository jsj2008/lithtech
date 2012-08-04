// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalGetBackup.h
//
// PURPOSE : AIGoalGetBackup class definition
//
// CREATED : 7/30/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIGOAL_GETBACKUP_H__
#define __AIGOAL_GETBACKUP_H__

#include "AIGoalAbstractSearch.h"

// Forward declarations.
class AINode;
enum  EnumAIStimulusID;

class CAIGoalGetBackup : public CAIGoalAbstractSearch
{
	typedef CAIGoalAbstractSearch super;

	public:

		DECLARE_AI_FACTORY_CLASS_SPECIFIC(Goal, CAIGoalGetBackup, kGoal_GetBackup);

		CAIGoalGetBackup( );

		// Save / Load

		virtual void	Save(ILTMessage_Write *pMsg);
        virtual void	Load(ILTMessage_Read *pMsg);

		// Activation.

		virtual void ActivateGoal();
		virtual void DeactivateGoal();

		// Updating.

		void UpdateGoal();

		// Attractor Handling.

		virtual AINode*	HandleGoalAttractors();

		// Triggered AI Sounds.

		virtual LTBOOL SelectTriggeredAISound(EnumAISoundType* peAISoundType);

		// Sense Handling.

		virtual LTBOOL HandleGoalSenseTrigger(AISenseRecord* pSenseRecord);

	protected:

		// State Handling.

		void HandleStateGetBackup();
		void HandleStateGoto();

	protected:

		EnumAIStimulusID	m_eStimulusID;
		LTObjRef			m_hNodeBackup;
		LTObjRef			m_hFailedNode;
		LTVector			m_vEnemySeenPos;
};


#endif
