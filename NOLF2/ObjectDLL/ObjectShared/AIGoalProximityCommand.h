// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalProximityCommand.h
//
// PURPOSE : AIGoalProximityCommand class definition
//
// CREATED : 6/17/02
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIGOAL_PROXIMITY_COMMAND_H__
#define __AIGOAL_PROXIMITY_COMMAND_H__

#include "AIGoalAbstractStimulated.h"


class CAIGoalProximityCommand : public CAIGoalAbstractStimulated
{
	typedef CAIGoalAbstractStimulated super;

	public:

		DECLARE_AI_FACTORY_CLASS_SPECIFIC(Goal, CAIGoalProximityCommand, kGoal_ProximityCommand);

		CAIGoalProximityCommand( );

		// Initialization.

		virtual void InitGoal(CAI* pAI, LTFLOAT fImportance, LTFLOAT fTime);

		// Save / Load

		virtual void	Save(ILTMessage_Write *pMsg);
        virtual void	Load(ILTMessage_Read *pMsg);

		// Sense Handling.

		virtual LTBOOL HandleGoalSenseTrigger(AISenseRecord* pSenseRecord);

		// Command Handling.

		virtual LTBOOL HandleNameValuePair(const char *szName, const char *szValue);

	protected:

		LTFLOAT		m_fProximityDistSqr;
		LTBOOL		m_bSentCommand;
};


#endif
