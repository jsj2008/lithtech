// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalFlushOut.h
//
// PURPOSE : AIGoalFlushOut class definition
//
// CREATED : 4/11/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIAMGOAL_FLUSH_OUT_H__
#define __AIAMGOAL_FLUSH_OUT_H__

#include "AIGoalAbstract.h"


class CAIGoalFlushOut : public CAIGoalAbstract
{
	typedef CAIGoalAbstract super;

	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( Goal, CAIGoalFlushOut, kGoal_FlushOut );

				 CAIGoalFlushOut();
		virtual ~CAIGoalFlushOut();

		virtual void	CalculateGoalRelevance();

		virtual void	ActivateGoal();

		virtual void	SetWSSatisfaction( CAIWorldState& WorldState );
		virtual bool	IsWSSatisfied( CAIWorldState* pwsWorldState );

	protected:
};

#endif
