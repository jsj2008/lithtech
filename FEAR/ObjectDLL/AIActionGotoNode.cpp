// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionGotoNode.cpp
//
// PURPOSE : AIActionGotoNode abstract class implementation
//
// CREATED : 1/29/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIActionGotoNode.h"
#include "AI.h"
#include "AINode.h"
#include "AIState.h"
#include "AIStateGoto.h"
#include "AIBlackBoard.h"
#include "AIPathMgrNavMesh.h"
#include "NodeTrackerContext.h"
#include "AnimationContext.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionGotoNode, kAct_GotoNode );


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionGotoNode::Con/destructor
//
//	PURPOSE:	Con/destructor
//
// ----------------------------------------------------------------------- //

CAIActionGotoNode::CAIActionGotoNode()
{
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionGotoNode::InitAction
//
//	PURPOSE:	Handle initializing the AIAction, setting the 
//				ActionRecord to use, as well as preconditions and effects.
//
// ----------------------------------------------------------------------- //

void CAIActionGotoNode::InitAction( AIDB_ActionRecord* pActionRecord )
{
	super::InitAction( pActionRecord );

	// The node to goto is a variable. Which node depends on the goal 
	// or action we are trying to satisfy.

	// No preconditions.

	// Set effects.
	// At the node.

	m_wsWorldStateEffects.SetWSProp( kWSK_AtNode, NULL, kWST_Variable, kWSK_AtNode );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionGotoNode::ValidateContextPreconditions
//
//	PURPOSE:	Return true if real-time preconditions are valid.
//
// ----------------------------------------------------------------------- //

bool CAIActionGotoNode::ValidateContextPreconditions( CAI* pAI, CAIWorldState& wsWorldStateGoal, bool bIsPlanning )
{
	// Only check path if planning.

	if( !bIsPlanning )
	{
		return true;
	}

	// Find which node we are going to from the goal world state.

	SAIWORLDSTATE_PROP* pProp = wsWorldStateGoal.GetWSProp( kWSK_AtNode, pAI->m_hObject );
	if( !pProp )
	{
		return false;
	}

	AINode* pNode = (AINode*)g_pLTServer->HandleToObject( pProp->hWSValue );
	if( !pNode )
	{
		return false;
	}

	// Failed to find a destination position for this node.

	LTVector vPos;
	if (!pNode->GetDestinationPosition(pAI, pAI->GetAIBlackBoard()->GetBBTargetPosition(), vPos))
	{
		return false;
	}

	// Determine if a path exists to the node.

	return g_pAIPathMgrNavMesh->HasPath( pAI, pAI->GetCharTypeMask(), vPos );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionGotoNode::ActivateAction
//
//	PURPOSE:	Activate action.
//
// ----------------------------------------------------------------------- //

void CAIActionGotoNode::ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal )
{
	super::ActivateAction( pAI, wsWorldStateGoal );

	// Find which node we are going to from the goal world state.

	SAIWORLDSTATE_PROP* pProp = wsWorldStateGoal.GetWSProp( kWSK_AtNode, pAI->m_hObject );
	if( pProp )
	{
		// Set the Goto state.

		pAI->SetState( kState_Goto );

		// Set the destination node.

		CAIStateGoto* pGoto = (CAIStateGoto*)pAI->GetState();
		pGoto->SetDestNode( pProp->hWSValue );

		// Torso tracking.

		if( pAI->GetAIBlackBoard()->GetBBAwareness() == kAware_Alert )
		{
			// Aim ranged weapon while running.

			if( AIWeaponUtils::HasWeaponType( pAI, kAIWeaponType_Ranged, !AIWEAP_CHECK_HOLSTER ) 
				&& AIWeaponUtils::HasAmmo( pAI, kAIWeaponType_Ranged, !AIWEAP_CHECK_HOLSTER ) )
			{
				pAI->GetAIBlackBoard()->SetBBTargetTrackerFlags( kTrackerFlag_AimAt );
			}

			// Look at target while running.

			else {
				pAI->GetAIBlackBoard()->SetBBTargetTrackerFlags( kTrackerFlag_LookAt );
			}
		}
		else {
			pAI->GetAIBlackBoard()->SetBBTargetTrackerFlags( kTrackerFlag_None );
		}
		pAI->GetAIBlackBoard()->SetBBFaceTarget( false );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionGotoNode::IsActionComplete
//
//	PURPOSE:	Determine if action has completed.
//
// ----------------------------------------------------------------------- //

bool CAIActionGotoNode::IsActionComplete( CAI* pAI )
{
	// Allow transitions to finish.

	if( pAI->GetAnimationContext()->IsTransitioning() )
	{
		return false;
	}

	// Goto is complete if state is complete.

	if( ( pAI->GetState() ) &&
		( pAI->GetState()->GetStateStatus() == kAIStateStatus_Complete ) )
	{
		return true;
	}

	// Goto is not complete.

	return false;
}

