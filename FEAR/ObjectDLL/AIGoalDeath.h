// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalDeath.h
//
// PURPOSE : AIGoalDeath class definition
//
// CREATED : 04/15/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIGOAL_DEATH_H__
#define __AIGOAL_DEATH_H__

#include "AIGoalAbstract.h"


// Forward declarations.


// ----------------------------------------------------------------------- //

class CAIGoalDeath : public CAIGoalAbstract
{
	typedef CAIGoalAbstract super;

	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( Goal, CAIGoalDeath, kGoal_Death );
		
		CAIGoalDeath();
		virtual ~CAIGoalDeath();

		virtual void	Save(ILTMessage_Write *pMsg);
		virtual void	Load(ILTMessage_Read *pMsg);

		void			StartDeath();

		// CAIGoalAbstract overrides.

		virtual void		CalculateGoalRelevance();
		virtual void		HandleBuildPlanFailure();
		virtual void		ActivateGoal();
		virtual void		DeactivateGoal();
		virtual bool		UpdateGoal();
		virtual bool		IsPlanValid();

		virtual void		SetWSSatisfaction( CAIWorldState& WorldState );
		virtual bool		IsWSSatisfied( CAIWorldState* pwsWorldState );
};


#endif
