// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalRetreat.h
//
// PURPOSE : AIGoalRetreat class definition
//
// CREATED : 4/24/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIAMGOAL_RETREAT_H__
#define __AIAMGOAL_RETREAT_H__

#include "AIGoalUseSmartObjectCombat.h"

// Forward declarations.


// ----------------------------------------------------------------------- //

class CAIGoalRetreat : public CAIGoalUseSmartObjectCombat
{
	typedef CAIGoalUseSmartObjectCombat super;

	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( Goal, CAIGoalRetreat, kGoal_Retreat );
		
		CAIGoalRetreat();

		// CAIGoalAbstract overrides.

		virtual void		CalculateGoalRelevance();
		virtual HOBJECT		FindBestNode( EnumAINodeType eNodeType );
		virtual void		DeactivateGoal();
};


#endif
