// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionFollowPlayer.h
//
// PURPOSE : AIActionFollowPlayer class definition
//
// CREATED : 5/02/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIACTION_FOLLOW_PLAYER_H__
#define __AIACTION_FOLLOW_PLAYER_H__

#include "AIActionFollow.h"

class CAIActionFollowPlayer : public CAIActionFollow
{
	typedef CAIActionFollow super;
	
	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionFollowPlayer, kAct_FollowPlayer );

		// CAIActionAbstract members.

		virtual bool	ValidateContextPreconditions( CAI* pAI, CAIWorldState& wsWorldStateGoal, bool bIsPlanning );
};

// ----------------------------------------------------------------------- //

#endif
