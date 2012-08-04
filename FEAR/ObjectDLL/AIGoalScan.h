// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalScan.h
//
// PURPOSE : AIGoalScan class definition
//
// CREATED : 9/18/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIGOAL_SCAN_H__
#define __AIGOAL_SCAN_H__

#include "AIGoalPatrol.h"



// ----------------------------------------------------------------------- //

class CAIGoalScan : public CAIGoalPatrol
{
	typedef CAIGoalPatrol super;

	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( Goal, CAIGoalScan, kGoal_Scan );

		CAIGoalScan();
		virtual ~CAIGoalScan();

        virtual void	Save(ILTMessage_Write *pMsg);
        virtual void	Load(ILTMessage_Read *pMsg);

		virtual void	ActivateGoal();
		virtual void	DeactivateGoal();
		virtual bool	ReplanRequired();

	protected:

		virtual void	AssignTargetPatrolNode( CAIWMFact* pFact );
		virtual HOBJECT	GetNextPatrolNode();
};


#endif
