// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalUseSmartObjectCombat.h
//
// PURPOSE : AIGoalUseSmartObjectCombat class definition
//
// CREATED : 2/28/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIAMGOAL_USE_SMART_OBJECT_COMBAT_H__
#define __AIAMGOAL_USE_SMART_OBJECT_COMBAT_H__

#include "AIGoalUseSmartObject.h"

// Forward declarations.


// ----------------------------------------------------------------------- //

class CAIGoalUseSmartObjectCombat : public CAIGoalUseSmartObject
{
	typedef CAIGoalUseSmartObject super;

	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( Goal, CAIGoalUseSmartObjectCombat, kGoal_UseSmartObjectCombat );
		
		// CAIGoalUseSmartObject overrides.

		virtual bool		IsGoalInProgress();
		virtual HOBJECT		FindBestNode( EnumAINodeType eNodeType );

		// CAIGoalAbstract overrides.

		virtual void		CalculateGoalRelevance();
		virtual void		ActivateGoal();
		virtual void		SetWSSatisfaction( CAIWorldState& WorldState );
};


#endif
