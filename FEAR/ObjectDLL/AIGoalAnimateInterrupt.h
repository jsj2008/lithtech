// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalAnimateInterrupt.h
//
// PURPOSE : AIGoalAnimateInterrupt class definition
//
// CREATED : 9/4/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIAMGOAL_ANIMATE_INTERRUPT_H__
#define __AIAMGOAL_ANIMATE_INTERRUPT_H__

#include "AIGoalAnimate.h"

// Forward declarations.


// ----------------------------------------------------------------------- //

class CAIGoalAnimateInterrupt : public CAIGoalAnimate
{
	typedef CAIGoalAnimate super;

	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( Goal, CAIGoalAnimateInterrupt, kGoal_AnimateInterrupt );

	protected:
		virtual CAIWMFact* GetAnimateTaskFact();
		virtual CAIWMFact* GetAnimateLoopTaskFact();
};

#endif // __AIAMGOAL_ANIMATE_INTERRUPT_H__
