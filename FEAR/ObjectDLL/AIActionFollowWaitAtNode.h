// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionFollowWaitAtNode.h
//
// PURPOSE : AIActionFollowWaitAtNode class definition
//
// CREATED : 07/19/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIACTION_FOLLOW_WAIT_AT_NODE_H__
#define __AIACTION_FOLLOW_WAIT_AT_NODE_H__

#include "AIActionUseSmartObjectNode.h"


class CAIActionFollowWaitAtNode : public CAIActionUseSmartObjectNode
{
	typedef CAIActionUseSmartObjectNode super;

	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionFollowWaitAtNode, kAct_FollowWaitAtNode );

		CAIActionFollowWaitAtNode();

		// CAIActionAbstract members.

		virtual void	SetPlanWSPreconditions( CAI* pAI, CAIWorldState& wsWorldStateGoal );
		virtual bool	ValidateContextPreconditions( CAI* pAI, CAIWorldState& wsWorldStateGoal, bool bIsPlanning );
		virtual void	ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal );
};

// ----------------------------------------------------------------------- //

#endif
