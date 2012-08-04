// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalSearchFollow.h
//
// PURPOSE : AIGoalSearchFollow class definition
//
// CREATED : 9/21/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIGOAL_SEARCH_FOLLOW_H__
#define __AIGOAL_SEARCH_FOLLOW_H__

#include "AIGoalFollow.h"


// ----------------------------------------------------------------------- //

class CAIGoalSearchFollow : public CAIGoalFollow
{
	typedef CAIGoalFollow super;

	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( Goal, CAIGoalSearchFollow, kGoal_SearchFollow );
		
		// CAIGoalAbstract overrides.

		virtual void		CalculateGoalRelevance();
};


#endif
