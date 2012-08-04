// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionGotoNodeCovered.cpp
//
// PURPOSE : AIActionGotoNodeCovered class implementation
//
// CREATED : 02/09/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIActionGotoNodeCovered.h"
#include "AI.h"
#include "AINode.h"
#include "AIStateGoto.h"
#include "AIBlackBoard.h"
#include "AnimationContext.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionGotoNodeCovered, kAct_GotoNodeCovered );


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionGotoNodeCovered::Con/destructor
//
//	PURPOSE:	Con/destructor
//
// ----------------------------------------------------------------------- //

CAIActionGotoNodeCovered::CAIActionGotoNodeCovered()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionGotoNodeCovered::ValidateContextPreconditions
//
//	PURPOSE:	Return true if real-time preconditions are valid.
//
// ----------------------------------------------------------------------- //

bool CAIActionGotoNodeCovered::ValidateContextPreconditions( CAI* pAI, CAIWorldState& wsWorldStateGoal, bool bIsPlanning )
{
	if( !super::ValidateContextPreconditions( pAI, wsWorldStateGoal, bIsPlanning ) )
	{
		return false;
	}

	// AI is engaged in a cover behavior.

	EnumAnimProp eActivity = pAI->GetAnimationContext()->GetCurrentProp( kAPG_Activity );
	if( !( ( eActivity == kAP_ATVT_Covered ) ||
		   ( eActivity == kAP_ATVT_Uncovered ) ||
		   ( eActivity == kAP_ATVT_Blind ) ) )
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

		// Bail if not heading to a cover node.

		AINode* pNode = (AINode*)g_pLTServer->HandleToObject( pProp->hWSValue );
		if( !( pNode && pNode->GetType() == kNode_Cover ) )
		{
			return false;
		}
	}

	// Action is valid.

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionGotoNodeCovered::ActivateAction
//
//	PURPOSE:	Activate action.
//
// ----------------------------------------------------------------------- //

void CAIActionGotoNodeCovered::ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal )
{
	super::ActivateAction( pAI, wsWorldStateGoal );

	// Sanity check.

	if( pAI->GetState()->GetStateClassType() != kState_Goto )
	{
		return;
	}

	// Set the additional anim props for running to cover.

	CAIStateGoto* pStateGoto = (CAIStateGoto*)( pAI->GetState() );
	if( pStateGoto )
	{
		pStateGoto->SetActivityProp( kAP_ATVT_Uncovered );
	}
}

