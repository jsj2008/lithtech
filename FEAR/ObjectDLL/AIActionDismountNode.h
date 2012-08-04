// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionDismountNode.h
//
// PURPOSE : AIActionDismountNode class definition
//
// CREATED : 04/22/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIACTION_DISMOUNT_NODE_H__
#define __AIACTION_DISMOUNT_NODE_H__

#include "AIActionUseSmartObjectNode.h"


class CAIActionDismountNode : public CAIActionUseSmartObjectNode
{
	typedef CAIActionUseSmartObjectNode super;

	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionDismountNode, kAct_DismountNode );

		CAIActionDismountNode();

		// CAIActionAbstract members.

		virtual void			InitAction( AIDB_ActionRecord* pActionRecord );
		virtual bool			ValidateWSPreconditions( CAI* pAI, CAIWorldState& wsWorldStateCur, CAIWorldState& wsWorldStateGoal, ENUM_AIWORLDSTATE_PROP_KEY* pFailedWSK );
		virtual void			ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal );
		virtual bool			ValidateAction( CAI* pAI );
		virtual void			ApplyContextEffect( CAI* pAI, CAIWorldState* pwsWorldStateCur, CAIWorldState* pwsWorldStateGoal );

		virtual EnumAnimProp	GetDismountAction() { return kAP_ACT_Dismount; }
};

// ----------------------------------------------------------------------- //

#endif
