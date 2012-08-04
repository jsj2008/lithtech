// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalGuard.h
//
// PURPOSE : AIGoalGuard class definition
//
// CREATED : 4/07/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIGOAL_GUARD_H__
#define __AIGOAL_GUARD_H__

#include "AIGoalAbstract.h"


// Forward declarations.


// ----------------------------------------------------------------------- //

class CAIGoalGuard : public CAIGoalAbstract
{
	typedef CAIGoalAbstract super;

	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( Goal, CAIGoalGuard, kGoal_Guard );

		CAIGoalGuard();
		virtual ~CAIGoalGuard();

        virtual void	Save(ILTMessage_Write *pMsg);
        virtual void	Load(ILTMessage_Read *pMsg);

		virtual void	CalculateGoalRelevance();
		virtual bool	ReplanRequired();

		virtual void	ActivateGoal();
		virtual void	DeactivateGoal();

		virtual void	SetWSSatisfaction( CAIWorldState& WorldState );
		virtual bool	IsWSSatisfied( CAIWorldState* pwsWorldState );

	protected:

		HOBJECT		m_hNodeGuard;
};


#endif
