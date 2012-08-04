// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionGotoNodeDirect.h
//
// PURPOSE : Contains the declaration of the GotoNodeDirect action.  
//				This action causes the AI to go to the node without any 
//				fancy extras such as direcitonal movement
//
// CREATED : 4/08/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIACTIONGOTONODEDIRECT_H_
#define __AIACTIONGOTONODEDIRECT_H_

#include "AIActionGotoNode.h"

class CAIActionGotoNodeDirect : public CAIActionGotoNode
{
	typedef CAIActionGotoNode super;

	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionGotoNodeDirect, kAct_GotoNodeDirect );

		CAIActionGotoNodeDirect();

		// CAIActionAbstract members.

		virtual bool	ValidateContextPreconditions( CAI* pAI, CAIWorldState& wsWorldStateGoal, bool bIsPlanning );
		virtual void	ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal );
		virtual void	DeactivateAction( CAI* pAI );
};

// ----------------------------------------------------------------------- //

#endif // __AIACTIONGOTONODEDIRECT_H_

