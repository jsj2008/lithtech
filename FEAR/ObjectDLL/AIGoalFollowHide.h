// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalFollowHide.h
//
// PURPOSE : AIGoalFollowHide class definition
//
// CREATED : 07/19/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIGOAL_FOLLOW_HIDE_H__
#define __AIGOAL_FOLLOW_HIDE_H__

#include "AIGoalUseSmartObjectCombat.h"


// ----------------------------------------------------------------------- //

class CAIGoalFollowHide : public CAIGoalUseSmartObjectCombat
{
	typedef CAIGoalUseSmartObjectCombat super;

	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( Goal, CAIGoalFollowHide, kGoal_FollowHide );
		
		CAIGoalFollowHide();

		// CAIGoalUseSmartObject overrides.

		virtual void		Save(ILTMessage_Write *pMsg);
        virtual void		Load(ILTMessage_Read *pMsg);

		// CAIGoalAbstract overrides.

		virtual void		CalculateGoalRelevance();
		virtual void		SetWSSatisfaction( CAIWorldState& WorldState );
};


#endif
