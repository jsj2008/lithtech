// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalSearch.h
//
// PURPOSE : AIGoalSearch class definition
//
// CREATED : 8/19/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIGOAL_SEARCH_H__
#define __AIGOAL_SEARCH_H__

#include "AIGoalGoto.h"

// Forward declarations.


// ----------------------------------------------------------------------- //

class CAIGoalSearch : public CAIGoalGoto
{
	typedef CAIGoalGoto super;

	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( Goal, CAIGoalSearch, kGoal_Search );

		CAIGoalSearch();
		virtual ~CAIGoalSearch();

		// CAIGoalAbstract overrides.

		virtual void	Save(ILTMessage_Write *pMsg);
		virtual void	Load(ILTMessage_Read *pMsg);

		virtual void	SetWSSatisfaction( CAIWorldState& WorldState );
		virtual bool	IsWSSatisfied( CAIWorldState* pwsWorldState );
};


#endif
