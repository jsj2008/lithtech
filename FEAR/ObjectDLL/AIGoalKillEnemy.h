// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalKillEnemy.h
//
// PURPOSE : AIGoalKillEnemy class definition
//
// CREATED : 1/28/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIAMGOAL_KILL_ENEMY_H__
#define __AIAMGOAL_KILL_ENEMY_H__

#include "AIGoalAbstract.h"


class CAIGoalKillEnemy : public CAIGoalAbstract
{
	typedef CAIGoalAbstract super;

	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( Goal, CAIGoalKillEnemy, kGoal_KillEnemy );

				 CAIGoalKillEnemy();
		virtual ~CAIGoalKillEnemy();

		virtual void	CalculateGoalRelevance();
		virtual bool	ReplanRequired();

		virtual void	ActivateGoal();

		virtual void	SetWSSatisfaction( CAIWorldState& WorldState );
		virtual bool	IsWSSatisfied( CAIWorldState* pwsWorldState );

	protected:
};

#endif
