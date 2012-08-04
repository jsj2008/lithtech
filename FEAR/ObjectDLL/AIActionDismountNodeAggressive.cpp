// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionDismountNodeAggressive.cpp
//
// PURPOSE : Handles leaving a node in an aggressive way
//
// CREATED : 5/24/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIActionDismountNodeAggressive.h"
#include "AI.h"
#include "AINode.h"
#include "AIWorldState.h"
#include "AIBlackBoard.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionDismountNodeAggressive, kAct_DismountNodeAggressive );

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionDismountNodeAggressive::Con/destructor
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

CAIActionDismountNodeAggressive::CAIActionDismountNodeAggressive()
{
}

CAIActionDismountNodeAggressive::~CAIActionDismountNodeAggressive()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionDismountNodeAggressive::CAIActionDismountNode
//
//	PURPOSE:	Return true if this action's preconditions are met in 
//              plan's current world state.
//
// ----------------------------------------------------------------------- //

bool CAIActionDismountNodeAggressive::ValidateWSPreconditions( CAI* pAI, CAIWorldState& wsWorldStateCur, CAIWorldState& wsWorldStateGoal, ENUM_AIWORLDSTATE_PROP_KEY* pFailedWSK )
{
	if (!super::ValidateWSPreconditions( pAI, wsWorldStateCur, wsWorldStateGoal, pFailedWSK ))
	{
		return false;
	}

	// AI must already be at a node.

	SAIWORLDSTATE_PROP* pProp = pAI->GetAIWorldState()->GetWSProp( kWSK_AtNode, pAI->m_hObject );
	if( !pProp || !pProp->hWSValue)
	{
		if( pFailedWSK )
		{
			*pFailedWSK = pProp->eWSKey;
		}
		return false;
	}

	// Must be a valid node pointer.

	AINode* pNode = AINode::HandleToObject(pProp->hWSValue);
	if ( !pNode )
	{
		if( pFailedWSK )
		{
			*pFailedWSK = pProp->eWSKey;
		}
		return false;
	}

	// Node must be invalid due to a situation requiring an aggressive response.
	// Filter out any node status checks which invalid irrelevance or general failure.

	const int kNonHostileInvalidationClauseMask = kNodeStatus_TooFar 
		| kNodeStatus_Disabled
		| kNodeStatus_LockedByOther
		| kNodeStatus_ThreatOutsideBoundary
		| kNodeStatus_ThreatOutsideRegions
		| kNodeStatus_ThreatUnseen
		| kNodeStatus_SearchedRecently
		| kNodeStatus_RequiresPartner
		| kNodeStatus_Expired;

	if ( pNode->IsNodeValid( pAI, pAI->GetPosition(), 
		pAI->GetAIBlackBoard()->GetBBTargetObject(),
		kThreatPos_TargetPos,
		kNodeStatus_All ^ kNonHostileInvalidationClauseMask) )
	{
		return false;
	}

	return true;
}
