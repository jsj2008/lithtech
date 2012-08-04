// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalRespondToBackup.h
//
// PURPOSE : AIGoalRespondToBackup class definition
//
// CREATED : 11/15/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIGOAL_RESPONDTOBACKUP_H__
#define __AIGOAL_RESPONDTOBACKUP_H__

#include "AIGoalAbstractSearch.h"

// Forward declarations.
class AINode;


class CAIGoalRespondToBackup : public CAIGoalAbstractSearch
{
	typedef CAIGoalAbstractSearch super;

	public:

		DECLARE_AI_FACTORY_CLASS_SPECIFIC(Goal, CAIGoalRespondToBackup, kGoal_RespondToBackup);

		CAIGoalRespondToBackup( );

		// Save / Load

		virtual void	Save(ILTMessage_Write *pMsg);
        virtual void	Load(ILTMessage_Read *pMsg);

		// Activation.

		virtual void ActivateGoal();

		// Updating.

		void UpdateGoal();
		virtual void RecalcImportance();

		// Sense Handling.

		virtual LTBOOL HandleGoalSenseTrigger(AISenseRecord* pSenseRecord);

	protected:

		// State Handling.

		void HandleStateGoto();

	protected:

		LTObjRef	m_hNodeBackup;
		LTVector	m_vEnemySeenPos;
		uint32		m_cResponses;
		uint32		m_cArrivalCount;
};


#endif
