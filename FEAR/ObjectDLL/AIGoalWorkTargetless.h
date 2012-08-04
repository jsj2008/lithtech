// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalWorkTargetless.h
//
// PURPOSE : AIGoalWorkTargetless class definition
//
// CREATED : 9/07/04
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIAMGOAL_WORK_TARGETLESS_H__
#define __AIAMGOAL_WORK_TARGETLESS_H__

#include "AIGoalWork.h"

// ----------------------------------------------------------------------- //

class CAIGoalWorkTargetless : public CAIGoalWork
{
	typedef CAIGoalWork super;

	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( Goal, CAIGoalWorkTargetless, kGoal_WorkTargetless );
		
		CAIGoalWorkTargetless();

		// CAIGoalUseSmartObject overrides.

		virtual void	CalculateGoalRelevance();
};


#endif
