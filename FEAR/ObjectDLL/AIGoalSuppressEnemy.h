// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalSuppressEnemy.h
//
// PURPOSE : AIGoalSuppressEnemy class definition
//
// CREATED : 5/30/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIAMGOAL_SUPPRESS_ENEMY_H__
#define __AIAMGOAL_SUPPRESS_ENEMY_H__

#include "AIGoalAbstract.h"


class CAIGoalSuppressEnemy : public CAIGoalAbstract
{
	typedef CAIGoalAbstract super;

	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( Goal, CAIGoalSuppressEnemy, kGoal_SuppressEnemy );

				 CAIGoalSuppressEnemy();
		virtual ~CAIGoalSuppressEnemy();

		virtual void	CalculateGoalRelevance();

		virtual void	ActivateGoal();

		virtual bool	UpdateGoal();

		virtual void	SetWSSatisfaction( CAIWorldState& WorldState );
		virtual bool	IsWSSatisfied( CAIWorldState* pwsWorldState );

	protected:
};

#endif
