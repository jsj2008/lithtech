// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalSelfDefense.h
//
// PURPOSE : AIGoalSelfDefense class definition
//
// CREATED : 04/15/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIGOAL_SELF_DEFENSE_H__
#define __AIGOAL_SELF_DEFENSE_H__

#include "AIGoalKillEnemy.h"


class CAIGoalSelfDefense : public CAIGoalKillEnemy
{
	typedef CAIGoalKillEnemy super;

	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( Goal, CAIGoalSelfDefense, kGoal_SelfDefense );

				 CAIGoalSelfDefense();
		virtual ~CAIGoalSelfDefense();

		virtual void	CalculateGoalRelevance();
		virtual void	ActivateGoal();
};

#endif
