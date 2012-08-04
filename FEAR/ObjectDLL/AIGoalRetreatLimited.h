// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalRetreatLimited.h
//
// PURPOSE : AIGoalRetreatLimited class definition
//
// CREATED : 10/07/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIGOAL_RETREAT_LIMITED_H__
#define __AIGOAL_RETREAT_LIMITED_H__

#include "AIGoalRetreat.h"

// ----------------------------------------------------------------------- //

class CAIGoalRetreatLimited : public CAIGoalRetreat
{
	typedef CAIGoalRetreat super;

	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( Goal, CAIGoalRetreatLimited, kGoal_RetreatLimited );

		// CAIGoalAbstract overrides.

		virtual void		CalculateGoalRelevance();
///		virtual void		ActivateGoal();
		virtual void		DeactivateGoal();
};


#endif
