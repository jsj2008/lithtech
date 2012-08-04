// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionMountNodeUncloaked.h
//
// PURPOSE : AIActionMountNodeUncloaked class definition
//
// CREATED : 04/22/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIACTION_MOUNT_NODE_UNCLOAKED_H__
#define __AIACTION_MOUNT_NODE_UNCLOAKED_H__

#include "AIActionMountNode.h"


class CAIActionMountNodeUncloaked : public CAIActionMountNode
{
	typedef CAIActionMountNode super;

	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionMountNodeUncloaked, kAct_MountNodeUncloaked );

		CAIActionMountNodeUncloaked();

		// CAIActionAbstract members.

		virtual void			ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal );
		virtual void			DeactivateAction( CAI* pAI );
};

// ----------------------------------------------------------------------- //

#endif
