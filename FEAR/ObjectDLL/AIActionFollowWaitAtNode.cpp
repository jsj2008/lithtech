// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionFollowWaitAtNode.cpp
//
// PURPOSE : AIActionFollowWaitAtNode class implementation
//
// CREATED : 07/19/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIActionFollowWaitAtNode.h"
#include "AI.h"
#include "AIStateUseSmartObject.h"
#include "AIBlackBoard.h"
#include "AIWorkingMemory.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionFollowWaitAtNode, kAct_FollowWaitAtNode );


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionFollowWaitAtNode::Con/destructor
//
//	PURPOSE:	Con/destructor
//
// ----------------------------------------------------------------------- //

CAIActionFollowWaitAtNode::CAIActionFollowWaitAtNode()
{
	// Validate all node conditions, except expiration time.

	m_dwNodeStatusFlags = kNodeStatus_All & ~( kNodeStatus_Expired | kNodeStatus_ThreatLookingAtNode );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionFollowWaitAtNode::SetPlanWSPreconditions
//
//	PURPOSE:	Set this action's preconditions in plan's goal world state.
//
// ----------------------------------------------------------------------- //

void CAIActionFollowWaitAtNode::SetPlanWSPreconditions( CAI* pAI, CAIWorldState& wsWorldStateGoal )
{
	super::SetPlanWSPreconditions( pAI, wsWorldStateGoal );

	// Remove any armed requirement.  Allow AI to wait at any type of node.

	wsWorldStateGoal.ClearWSProp( kWSK_WeaponArmed, pAI->m_hObject );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionFollowWaitAtNode::ValidateContextPreconditions
//
//	PURPOSE:	Return true if real-time preconditions are valid.
//
// ----------------------------------------------------------------------- //

bool CAIActionFollowWaitAtNode::ValidateContextPreconditions( CAI* pAI, CAIWorldState& wsWorldStateGoal, bool bIsPlanning )
{
	if( !super::ValidateContextPreconditions( pAI, wsWorldStateGoal, bIsPlanning ) )
	{
		return false;
	}

	// Invalid if we have a ranged weapon.

	if( AIWeaponUtils::HasWeaponType( pAI, kAIWeaponType_Ranged, true ) )
	{
		return false;
	}

	// Invalid if we have no Follow task.

	CAIWMFact factQuery;
	factQuery.SetFactType( kFact_Task );
	factQuery.SetTaskType( kTask_Follow );
	CAIWMFact* pFact = pAI->GetAIWorkingMemory()->FindWMFact( factQuery );
	if( !pFact )
	{
		return false;
	}

	// Action is valid.

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionFollowWaitAtNode::ActivateAction
//
//	PURPOSE:	Activate action.
//
// ----------------------------------------------------------------------- //

void CAIActionFollowWaitAtNode::ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal )
{
	super::ActivateAction( pAI, wsWorldStateGoal );

	// Sanity check.

	if( pAI->GetState()->GetStateClassType() != kState_UseSmartObject )
	{
		return;
	}

	// Override any smart object action.
	// Idle at the node.

	CAIStateUseSmartObject* pStateUseSmartObject = (CAIStateUseSmartObject*)( pAI->GetState() );
	pStateUseSmartObject->SetProp( kAPG_Action, kAP_ACT_Idle );

	// Torso tracking.

	pAI->GetAIBlackBoard()->SetBBTargetTrackerFlags( kTrackerFlag_LookAt );
	pAI->GetAIBlackBoard()->SetBBFaceTarget( false );
}


