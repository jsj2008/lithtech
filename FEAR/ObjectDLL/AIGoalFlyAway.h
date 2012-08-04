// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalFlyAway.h
//
// PURPOSE : AIGoalFlyAway class definition
//
// CREATED : 02/04/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIGOAL_FLY_AWAY_H__
#define __AIGOAL_FLY_AWAY_H__

#include "AIGoalCritterFlee.h"


// Forward declarations.


// ----------------------------------------------------------------------- //

class CAIGoalFlyAway : public CAIGoalCritterFlee
{
	typedef CAIGoalCritterFlee super;

	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( Goal, CAIGoalFlyAway, kGoal_FlyAway );
		
		CAIGoalFlyAway();
		virtual ~CAIGoalFlyAway();

		// CAIGoalAbstract overrides.

		virtual void		ActivateGoal();
		virtual void		DeactivateGoal();
};


#endif
