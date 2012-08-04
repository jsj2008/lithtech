// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalCover.h
//
// PURPOSE : AIGoalCover class definition
//
// CREATED : 3/13/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIAMGOAL_COVER_H__
#define __AIAMGOAL_COVER_H__

#include "AIGoalGoto.h"

// Forward declarations.


// ----------------------------------------------------------------------- //

class CAIGoalCover : public CAIGoalGoto
{
	typedef CAIGoalGoto super;

	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( Goal, CAIGoalCover, kGoal_Cover );

		CAIGoalCover();
		virtual ~CAIGoalCover();

		// CAIGoalAbstract overrides.

		virtual void	Save(ILTMessage_Write *pMsg);
		virtual void	Load(ILTMessage_Read *pMsg);

		virtual void	CalculateGoalRelevance();
		virtual void	ActivateGoal();
		virtual void	DeactivateGoal();
		virtual void	SetWSSatisfaction( CAIWorldState& WorldState );

		virtual void	UpdateTaskStatus();
};


#endif
