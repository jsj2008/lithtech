// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalIntro.h
//
// PURPOSE : AIGoalIntro class definition
//
// CREATED : 2/08/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIGOAL_INTRO_H__
#define __AIGOAL_INTRO_H__

#include "AIGoalGoto.h"

// Forward declarations.


// ----------------------------------------------------------------------- //

class CAIGoalIntro : public CAIGoalGoto
{
	typedef CAIGoalGoto super;

	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( Goal, CAIGoalIntro, kGoal_Intro );

		CAIGoalIntro();
		virtual ~CAIGoalIntro();

		// CAIGoalAbstract overrides.

		virtual void	Save(ILTMessage_Write *pMsg);
		virtual void	Load(ILTMessage_Read *pMsg);

		virtual void	DeactivateGoal();
		virtual void	SetWSSatisfaction( CAIWorldState& WorldState );
		virtual bool	IsWSSatisfied( CAIWorldState* pwsWorldState );
};


#endif
