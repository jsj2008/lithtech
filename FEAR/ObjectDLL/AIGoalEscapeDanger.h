// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalEscapeDanger.h
//
// PURPOSE : Defines implementation of CAIGoalEscapeDanger
//
// CREATED : 5/02/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIGOAL_ESCAPE_DANGER_H_
#define __AIGOAL_ESCAPE_DANGER_H_

#include "AIGoalAbstract.h"
#include "AIWorkingMemory.h"

class CAIGoalEscapeDanger : public CAIGoalAbstract
{
	typedef CAIGoalAbstract super;

	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( Goal, CAIGoalEscapeDanger, kGoal_EscapeDanger );

		CAIGoalEscapeDanger();
		virtual ~CAIGoalEscapeDanger();

		// CAIGoalAbstract overrides.

		virtual void	CalculateGoalRelevance();

		virtual void	ActivateGoal();
		virtual void	DeactivateGoal();

		virtual void	HandleBuildPlanFailure();

		virtual void	SetWSSatisfaction( CAIWorldState& WorldState );
		virtual bool	IsWSSatisfied( CAIWorldState* pwsWorldState );

	protected:

		void			AvoidCoverNode();
};


#endif // __AIGOALCORRECTPOSITION_H_
