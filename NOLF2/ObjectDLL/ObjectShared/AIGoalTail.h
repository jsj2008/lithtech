// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalTail.h
//
// PURPOSE : AIGoalTail class definition
//
// CREATED : 7/23/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIGOAL_TAIL_H__
#define __AIGOAL_TAIL_H__

#include "AIGoalAbstractTargeted.h"


class CAIGoalTail : public CAIGoalAbstractTargeted
{
	typedef CAIGoalAbstractTargeted super;

	public:

		DECLARE_AI_FACTORY_CLASS_SPECIFIC(Goal, CAIGoalTail, kGoal_Tail);

		// Save / Load

		virtual void	Save(ILTMessage_Write *pMsg);
        virtual void	Load(ILTMessage_Read *pMsg);

		// Activation.

		virtual void ActivateGoal();

		// Updating.

		void UpdateGoal();

	protected:

		// State Handling.

		void HandleStateTail();
};


#endif
