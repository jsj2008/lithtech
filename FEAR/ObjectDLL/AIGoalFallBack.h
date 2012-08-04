// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalFallBack.h
//
// PURPOSE : AIGoalFallBack class definition
//
// CREATED : 10/10/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIAMGOAL_FALLBACK_H__
#define __AIAMGOAL_FALLBACK_H__

#include "AIGoalUseSmartObjectCombat.h"


// ----------------------------------------------------------------------- //

class CAIGoalFallBack : public CAIGoalUseSmartObjectCombat
{
	typedef CAIGoalUseSmartObjectCombat super;

	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( Goal, CAIGoalFallBack, kGoal_FallBack );
		
		CAIGoalFallBack();

		// CAIGoalUseSmartObject overrides.

		HOBJECT				FindBestNode( EnumAINodeType eNodeType );

		// CAIGoalAbstract overrides.

		virtual void		CalculateGoalRelevance();
};


#endif
