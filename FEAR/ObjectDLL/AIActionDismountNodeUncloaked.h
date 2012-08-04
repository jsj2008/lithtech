// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionDismountNodeUncloaked.h
//
// PURPOSE : AIActionDismountNodeUncloaked class definition
//
// CREATED : 04/22/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIACTION_DISMOUNT_NODE_UNCLOAKED_H__
#define __AIACTION_DISMOUNT_NODE_UNCLOAKED_H__

#include "AIActionDismountNode.h"


class CAIActionDismountNodeUncloaked : public CAIActionDismountNode
{
	typedef CAIActionDismountNode super;

	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionDismountNodeUncloaked, kAct_DismountNodeUncloaked );

		CAIActionDismountNodeUncloaked();

		// CAIActionAbstract members.

		virtual void			ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal );
		virtual void			DeactivateAction( CAI* pAI );
};

// ----------------------------------------------------------------------- //

#endif
