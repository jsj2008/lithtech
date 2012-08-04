// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalStunned.h
//
// PURPOSE : AIGoalStunned class definition
//
// CREATED : 09/12/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIGOAL_STUNNED_H__
#define __AIGOAL_STUNNED_H__

#include "AIGoalAbstract.h"


// ----------------------------------------------------------------------- //

class CAIGoalStunned : public CAIGoalAbstract
{
	typedef CAIGoalAbstract super;

	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( Goal, CAIGoalStunned, kGoal_Stunned );
		
		CAIGoalStunned();
		virtual ~CAIGoalStunned();

		virtual void	Save(ILTMessage_Write *pMsg);
		virtual void	Load(ILTMessage_Read *pMsg);

		// CAIGoalAbstract overrides.

		virtual void		CalculateGoalRelevance();
		virtual void		ActivateGoal();
		virtual void		DeactivateGoal();

		virtual void		SetWSSatisfaction( CAIWorldState& WorldState );
		virtual bool		IsWSSatisfied( CAIWorldState* pwsWorldState );
};


#endif
