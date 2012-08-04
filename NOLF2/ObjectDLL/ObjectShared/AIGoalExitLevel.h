// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalExitLevel.h
//
// PURPOSE : AIGoalExitLevel class definition
//
// CREATED : 3/16/02
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIGOAL_EXIT_LEVEL_H__
#define __AIGOAL_EXIT_LEVEL_H__

#include "AIGoalGuard.h"


class CAIGoalExitLevel : public CAIGoalGuard
{
	typedef CAIGoalGuard super;

	public:

		DECLARE_AI_FACTORY_CLASS_SPECIFIC(Goal, CAIGoalExitLevel, kGoal_ExitLevel);

		 CAIGoalExitLevel( );

		// Save / Load

		virtual void	Save(ILTMessage_Write *pMsg);
        virtual void	Load(ILTMessage_Read *pMsg);

		// Updating.

		virtual void	RecalcImportance();

	protected:

		// State Handling.

		virtual void	HandleStateGoto();

	protected:

		LTFLOAT		m_fExitAllowedTime;
};

#endif
