// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalIdle.h
//
// PURPOSE : AIGoalIdle class definition
//
// CREATED : 1/28/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIAMGOAL_IDLE_H__
#define __AIAMGOAL_IDLE_H__

#include "AIGoalAbstract.h"


class CAIGoalIdle : public CAIGoalAbstract
{
	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( Goal, CAIGoalIdle, kGoal_Idle );

		CAIGoalIdle();

		virtual void	CalculateGoalRelevance();
		virtual void	SetWSSatisfaction( CAIWorldState& WorldState );
		virtual bool	IsWSSatisfied( CAIWorldState* pwsWorldState );
};

#endif
