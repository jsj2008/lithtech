// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionGotoNode.h
//
// PURPOSE : AIActionGotoNode abstract class definition
//
// CREATED : 1/29/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIACTION_GOTO_NODE_H__
#define __AIACTION_GOTO_NODE_H__

#include "AIActionGotoAbstract.h"

class CAIActionGotoNode : public CAIActionGotoAbstract
{
	typedef CAIActionGotoAbstract super;
	
	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionGotoNode, kAct_GotoNode );

		CAIActionGotoNode();

		// CAIActionAbstract members.

		virtual void	InitAction( AIDB_ActionRecord* pActionRecord );
		virtual bool	ValidateContextPreconditions( CAI* pAI, CAIWorldState& wsWorldStateGoal, bool bIsPlanning );
		virtual void	ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal );
		virtual	bool	IsActionComplete( CAI* pAI );

	protected:

};

// ----------------------------------------------------------------------- //

#endif
