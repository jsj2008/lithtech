// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionGotoNodeCovered.h
//
// PURPOSE : AIActionGotoNodeCovered class definition
//
// CREATED : 02/09/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIACTION_GOTO_NODE_COVERED_H__
#define __AIACTION_GOTO_NODE_COVERED_H__

#include "AIActionGotoNode.h"

class CAIActionGotoNodeCovered : public CAIActionGotoNode
{
	typedef CAIActionGotoNode super;

	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionGotoNodeCovered, kAct_GotoNodeCovered );

		CAIActionGotoNodeCovered();

		// CAIActionAbstract members.

		virtual bool	ValidateContextPreconditions( CAI* pAI, CAIWorldState& wsWorldStateGoal, bool bIsPlanning );
		virtual void	ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal );
};

// ----------------------------------------------------------------------- //

#endif
