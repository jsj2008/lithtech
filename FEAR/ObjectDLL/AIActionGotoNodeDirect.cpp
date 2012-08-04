// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionGotoNodeDirect.cpp
//
// PURPOSE : Contains the implementation of the GotoNodeDirect action.  
//				This action causes the AI to go to the node without any 
//				fancy extras such as direcitonal movement
//
// CREATED : 4/08/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIActionGotoNodeDirect.h"
#include "AINode.h"
#include "AI.h"
#include "AIBlackBoard.h"
#include "AIState.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionGotoNodeDirect, kAct_GotoNodeDirect );


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionGotoNodeDirect::Con/destructor
//
//	PURPOSE:	Con/destructor
//
// ----------------------------------------------------------------------- //

CAIActionGotoNodeDirect::CAIActionGotoNodeDirect()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionGotoNodeDirect::ValidateContextPreconditions
//
//	PURPOSE:	Return true if real-time preconditions are valid.
//
// ----------------------------------------------------------------------- //

bool CAIActionGotoNodeDirect::ValidateContextPreconditions( CAI* pAI, CAIWorldState& wsWorldStateGoal, bool bIsPlanning )
{
	if( !super::ValidateContextPreconditions( pAI, wsWorldStateGoal, bIsPlanning ) )
	{
		return false;
	}

	// Only check node if planning.

	if( bIsPlanning )
	{
		// Find which node we are going to from the goal world state.

		SAIWORLDSTATE_PROP* pProp = wsWorldStateGoal.GetWSProp( kWSK_AtNode, pAI->m_hObject );
		if( !pProp )
		{
			return false;
		}

		// Bail if not heading to a ambush node.

		AINode* pNode = (AINode*)g_pLTServer->HandleToObject( pProp->hWSValue );
		if( pNode )
		{
			if (pNode->GetType() != kNode_Ambush 
				&& pNode->GetType() != kNode_Stalk)
			{
				return false;
			}
		}
	}

	// Action is valid.

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionGotoNodeDirect::ActivateAction
//
//	PURPOSE:	Activate action.
//
// ----------------------------------------------------------------------- //

void CAIActionGotoNodeDirect::ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal )
{
	super::ActivateAction( pAI, wsWorldStateGoal );

	// Sanity check.

	if( pAI->GetState()->GetStateClassType() != kState_Goto )
	{
		return;
	}


	pAI->GetAIBlackBoard()->SetBBAllowDirectionalRun(false);
	pAI->GetAIBlackBoard()->SetBBMovementFire(false);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionGotoNodeDirect::DeactivateAction
//
//	PURPOSE:	Deactivate action.
//
// ----------------------------------------------------------------------- //

void CAIActionGotoNodeDirect::DeactivateAction( CAI* pAI )
{
	pAI->GetAIBlackBoard()->SetBBAllowDirectionalRun(true);
	pAI->GetAIBlackBoard()->SetBBMovementFire(true);
}
