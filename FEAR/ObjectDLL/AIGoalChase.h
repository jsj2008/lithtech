// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalChase.h
//
// PURPOSE : AIGoalChase class definition
//
// CREATED : 3/13/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIAMGOAL_CHASE_H__
#define __AIAMGOAL_CHASE_H__

#include "AIGoalAbstract.h"


// Forward declarations.

class	CAIWMFact;


// ----------------------------------------------------------------------- //

class CAIGoalChase : public CAIGoalAbstract
{
	typedef CAIGoalAbstract super;

	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( Goal, CAIGoalChase, kGoal_Chase );
		
		CAIGoalChase();
		virtual ~CAIGoalChase();

		// CAIGoalAbstract overrides.

		virtual void		CalculateGoalRelevance();

		virtual void		ActivateGoal();

		virtual void		SetWSSatisfaction( CAIWorldState& WorldState );
		virtual bool		IsWSSatisfied( CAIWorldState* pwsWorldState );
};


#endif
