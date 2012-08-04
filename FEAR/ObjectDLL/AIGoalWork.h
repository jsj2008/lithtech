// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalWork.h
//
// PURPOSE : AIGoalWork class definition
//
// CREATED : 10/10/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIAMGOAL_WORK_H__
#define __AIAMGOAL_WORK_H__

#include "AIGoalUseSmartObject.h"
#include "AI.h"

// ----------------------------------------------------------------------- //

class CAIGoalWork : public CAIGoalUseSmartObject
{
	typedef CAIGoalUseSmartObject super;

	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( Goal, CAIGoalWork, kGoal_Work );
		
		CAIGoalWork();

		// CAIGoalUseSmartObject overrides.

		virtual void		CalculateGoalRelevance();
		virtual HOBJECT		FindBestNode( EnumAINodeType eNodeType );
};


#endif
