// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalCharge.h
//
// PURPOSE : AIGoalCharge class definition
//
// CREATED : 08/18/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIGOAL_CHARGE_H__
#define __AIGOAL_CHARGE_H__

#include "AIGoalKillEnemy.h"


class CAIGoalCharge : public CAIGoalKillEnemy
{
	typedef CAIGoalKillEnemy super;

	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( Goal, CAIGoalCharge, kGoal_Charge );

				 CAIGoalCharge();
		virtual ~CAIGoalCharge();

		virtual void	CalculateGoalRelevance();
};

#endif
