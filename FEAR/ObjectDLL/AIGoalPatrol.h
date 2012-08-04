// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalPatrol.h
//
// PURPOSE : AIGoalPatrol class definition
//
// CREATED : 1/28/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIGOAL_PATROL_H__
#define __AIGOAL_PATROL_H__

#include "AIGoalAbstract.h"


// Forward declarations.


// ----------------------------------------------------------------------- //

class CAIGoalPatrol : public CAIGoalAbstract
{
	typedef CAIGoalAbstract super;

	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( Goal, CAIGoalPatrol, kGoal_Patrol );

		CAIGoalPatrol();
		virtual ~CAIGoalPatrol();

        virtual void	Save(ILTMessage_Write *pMsg);
        virtual void	Load(ILTMessage_Read *pMsg);

		virtual void	CalculateGoalRelevance();

		virtual void	ActivateGoal();

		virtual void	SetWSSatisfaction( CAIWorldState& WorldState );
		virtual bool	IsWSSatisfied( CAIWorldState* pwsWorldState );

	protected:

		virtual void	AssignTargetPatrolNode( CAIWMFact* pFact );
		virtual HOBJECT	GetNextPatrolNode();

	protected:

		HOBJECT		m_hNodePatrol;
		HOBJECT		m_hNodePatrolLast;
		bool		bPatrolReverse;
};


#endif
