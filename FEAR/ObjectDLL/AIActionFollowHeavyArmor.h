// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionFollowHeavyArmor.h
//
// PURPOSE : AIActionFollowHeavyArmor class definition
//
// CREATED : 4/30/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIACTION_FOLLOW_HEAVY_ARMOR_H__
#define __AIACTION_FOLLOW_HEAVY_ARMOR_H__

#include "AIActionFollow.h"

class CAIActionFollowHeavyArmor : public CAIActionFollow
{
	typedef CAIActionFollow super;
	
	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionFollowHeavyArmor, kAct_FollowHeavyArmor );

		// CAIActionAbstract members.

		virtual bool	ValidateContextPreconditions( CAI* pAI, CAIWorldState& wsWorldStateGoal, bool bIsPlanning );
};

// ----------------------------------------------------------------------- //

#endif
