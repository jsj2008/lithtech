// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalAlert.h
//
// PURPOSE : AIGoalAlert class definition
//
// CREATED : 8/21/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIGOAL_ALERT_H__
#define __AIGOAL_ALERT_H__

#include "AIGoalAbstract.h"


// ----------------------------------------------------------------------- //

class CAIGoalAlert : public CAIGoalAbstract
{
	typedef CAIGoalAbstract super;

	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( Goal, CAIGoalAlert, kGoal_Alert );
		
		CAIGoalAlert();
		~CAIGoalAlert();

		// CAIGoalAbstract overrides.

		virtual void		Save(ILTMessage_Write *pMsg);
		virtual void		Load(ILTMessage_Read *pMsg);

		virtual void		CalculateGoalRelevance();

		virtual void		ActivateGoal();
		virtual void		DeactivateGoal();

		virtual void		SetWSSatisfaction( CAIWorldState& WorldState );
		virtual bool		IsWSSatisfied( CAIWorldState* pwsWorldState );
};


#endif
