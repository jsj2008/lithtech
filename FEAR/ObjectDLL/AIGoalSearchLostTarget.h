// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalSearchLostTarget.h
//
// PURPOSE : AIGoalSearchLostTarget class definition
//
// CREATED : 01/18/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIGOAL_SEARCH_LOST_TARGET_H__
#define __AIGOAL_SEARCH_LOST_TARGET_H__

#include "AIGoalAbstract.h"


class CAIGoalSearchLostTarget : public CAIGoalAbstract
{
	typedef CAIGoalAbstract super;

	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( Goal, CAIGoalSearchLostTarget, kGoal_SearchLostTarget );

				 CAIGoalSearchLostTarget();
		virtual ~CAIGoalSearchLostTarget();

		virtual void	CalculateGoalRelevance();
		virtual void	ActivateGoal();

		virtual void	SetWSSatisfaction( CAIWorldState& WorldState );
		virtual bool	IsWSSatisfied( CAIWorldState* pwsWorldState );

		virtual void	UpdateTaskStatus();

		virtual void	HandleBuildPlanFailure();

	protected:
};

#endif
