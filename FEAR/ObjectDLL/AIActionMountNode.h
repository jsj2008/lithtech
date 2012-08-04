// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionMountNode.h
//
// PURPOSE : AIActionMountNode class definition
//
// CREATED : 04/22/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIACTION_MOUNT_NODE_H__
#define __AIACTION_MOUNT_NODE_H__

#include "AIActionUseSmartObjectNode.h"


class CAIActionMountNode : public CAIActionUseSmartObjectNode
{
	typedef CAIActionUseSmartObjectNode super;

	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionMountNode, kAct_MountNode );

		CAIActionMountNode();

		// CAIActionAbstract members.

		virtual void			InitAction( AIDB_ActionRecord* pActionRecord );
		virtual void			ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal );
		virtual bool			ValidateAction( CAI* pAI );
		virtual void			ApplyContextEffect( CAI* pAI, CAIWorldState* pwsWorldStateCur, CAIWorldState* pwsWorldStateGoal );
};

// ----------------------------------------------------------------------- //

#endif
