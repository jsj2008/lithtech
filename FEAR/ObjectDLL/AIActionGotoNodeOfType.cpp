// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionGotoNodeOfType.cpp
//
// PURPOSE : AIActionGotoNodeOfType abstract class implementation
//
// CREATED : 10/07/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIActionGotoNodeOfType.h"
#include "AI.h"
#include "AINode.h"
#include "AIState.h"
#include "AIStateGoto.h"
#include "AIBlackBoard.h"
#include "AITarget.h"
#include "AIPathMgrNavMesh.h"
#include "AIWorkingMemory.h"
#include "NodeTrackerContext.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionGotoNodeOfType, kAct_GotoNodeOfType );


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionGotoNodeOfType::Con/destructor
//
//	PURPOSE:	Con/destructor
//
// ----------------------------------------------------------------------- //

CAIActionGotoNodeOfType::CAIActionGotoNodeOfType()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionGotoNodeOfType::InitAction
//
//	PURPOSE:	Handle initializing the AIAction, setting the 
//				ActionRecord to use, as well as preconditions and effects.
//
// ----------------------------------------------------------------------- //

void CAIActionGotoNodeOfType::InitAction( AIDB_ActionRecord* pActionRecord )
{
	super::InitAction( pActionRecord );

	// The node type to goto is a variable. Which node type depends on the goal 
	// or action we are trying to satisfy.

	// No preconditions.

	// Set effects.
	// At a node of desired type.

	m_wsWorldStateEffects.SetWSProp( kWSK_AtNodeType, NULL, kWST_Variable, kWSK_AtNodeType );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionGotoNodeOfType::ValidateContextPreconditions
//
//	PURPOSE:	Return true if real-time preconditions are valid.
//
// ----------------------------------------------------------------------- //

bool CAIActionGotoNodeOfType::ValidateContextPreconditions( CAI* pAI, CAIWorldState& wsWorldStateGoal, bool bIsPlanning )
{
	// Only check path if planning.

	if( !bIsPlanning )
	{
		return true;
	}

	// Find which node type we are going to from the goal world state.

	SAIWORLDSTATE_PROP* pProp = wsWorldStateGoal.GetWSProp( kWSK_AtNodeType, pAI->m_hObject );
	if( !pProp )
	{
		return false;
	}

	// Find a node of the specified type that the AI knows about.

	EnumAINodeType eNodeType = pProp->eAINodeTypeWSValue;
	AINode* pNode = FindNodeOfType( pAI, eNodeType );
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
//	ROUTINE:	CAIActionGotoNodeOfType::ApplyWSEffect
//
//	PURPOSE:	Apply effects of the action to the current world state.
//
// ----------------------------------------------------------------------- //

void CAIActionGotoNodeOfType::ApplyWSEffect( CAI* pAI, CAIWorldState* pwsWorldStateCur, CAIWorldState* pwsWorldStateGoal )
{
	// Sanity check.

	if( !pAI )
	{
		return;
	}

	super::ApplyWSEffect( pAI, pwsWorldStateCur, pwsWorldStateGoal );

	// Find which node type we are going to from the goal world state.

	SAIWORLDSTATE_PROP* pProp = pwsWorldStateGoal->GetWSProp( kWSK_AtNodeType, pAI->m_hObject );
	if( !pProp )
	{
		return;
	}

	// Find a node of the specified type that the AI knows about.

	EnumAINodeType eNodeType = pProp->eAINodeTypeWSValue;
	AINode* pNode = FindNodeOfType( pAI, eNodeType );
	if( !pNode )
	{
		return;
	}

	// Set the effect of being at the specified node.

	pwsWorldStateCur->SetWSProp( kWSK_AtNode, NULL, kWST_HOBJECT, pNode->m_hObject );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionGotoNodeOfType::ValidateContextPreconditions
//
//	PURPOSE:	Return true if real-time preconditions are valid.
//
// ----------------------------------------------------------------------- //

AINode* CAIActionGotoNodeOfType::FindNodeOfType( CAI* pAI, EnumAINodeType eNodeType )
{
	// Sanity check.

	if( !pAI )
	{
		return NULL;
	}

	// Find AI's current threat.

	HOBJECT hThreat = NULL;
	if( pAI->HasTarget( kTarget_Character | kTarget_Object | kTarget_CombatOpportunity ) )
	{
		hThreat = pAI->GetAIBlackBoard()->GetBBTargetObject();
	}

	// Find the best node to go to.

	CAIWMFact* pFact = pAI->GetAIWorkingMemory()->FindFactNodeMax( pAI, eNodeType, kNodeStatus_All, NULL, hThreat );
	if( !pFact )
	{
		return NULL;
	}
	return (AINode*)g_pLTServer->HandleToObject( pFact->GetTargetObject() );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionGotoNodeOfType::ValidateWSEffects
//
//	PURPOSE:	Return true if this action's effects are not already met in 
//              plan's current world state.
//
// ----------------------------------------------------------------------- //

bool CAIActionGotoNodeOfType::ValidateWSEffects( CAI* pAI, CAIWorldState& wsWorldStateCur, CAIWorldState& wsWorldStateGoal )
{
	// Sanity check.

	if( !pAI )
	{
		return false;
	}

	// The super:: will fail to validate the effect if the AI is 
	// already at a node of desired type.

	if( !super::ValidateWSEffects( pAI, wsWorldStateCur, wsWorldStateGoal ) )
	{
		// If the AI is at a node, ensure it is valid.

		SAIWORLDSTATE_PROP* pProp = pAI->GetAIWorldState()->GetWSProp( kWSK_AtNode, pAI->m_hObject );
		if( pProp )
		{
			AINode* pNode = (AINode*)g_pLTServer->HandleToObject( pProp->hWSValue );
			if( !pNode )
			{
				return true;
			}

			// The node is invalid, so the Effect of this Action IS valid.

			if( !pNode->IsNodeValid( pAI, pAI->GetPosition(), pAI->GetAIBlackBoard()->GetBBTargetObject(), kThreatPos_TargetPos, kNodeStatus_All ) )
			{
				return true;
			}
		}

		// The Effect is not valid.

		return false;
	}

	// The Effect is valid.

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionGotoNodeOfType::ActivateAction
//
//	PURPOSE:	Activate action.
//
// ----------------------------------------------------------------------- //

void CAIActionGotoNodeOfType::ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal )
{
	super::ActivateAction( pAI, wsWorldStateGoal );

	// Find which node type we are going to from the goal world state.

	SAIWORLDSTATE_PROP* pProp = wsWorldStateGoal.GetWSProp( kWSK_AtNodeType, pAI->m_hObject );
	if( !pProp )
	{
		return;
	}
	EnumAINodeType eNodeType = pProp->eAINodeTypeWSValue;

	// Find AI's current threat.

	HOBJECT hThreat = NULL;
	if( pAI->HasTarget( kTarget_Character | kTarget_Object | kTarget_CombatOpportunity) )
	{
		hThreat = pAI->GetAIBlackBoard()->GetBBTargetObject();
	}

	// Find the best node to go to.

	CAIWMFact* pFact = pAI->GetAIWorkingMemory()->FindFactNodeMax( pAI, eNodeType, kNodeStatus_All, NULL, hThreat );
	if( !pFact )
	{
		return;
	}
	AINode* pNode = (AINode*)g_pLTServer->HandleToObject( pFact->GetTargetObject() );
	if( !pNode )
	{
		return;
	}

	// Set the Goto state.

	pAI->SetState( kState_Goto );

	// Set the destination node.

	CAIStateGoto* pGoto = (CAIStateGoto*)pAI->GetState();
	pGoto->SetDestNode( pNode->m_hObject );

	// Torso tracking.

	if( pAI->GetAIBlackBoard()->GetBBAwareness() == kAware_Alert )
	{
		pAI->GetAIBlackBoard()->SetBBTargetTrackerFlags( kTrackerFlag_AimAt );
	}
	else {
		pAI->GetAIBlackBoard()->SetBBTargetTrackerFlags( kTrackerFlag_None );
	}
	pAI->GetAIBlackBoard()->SetBBFaceTarget( false );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionGotoNodeOfType::IsActionComplete
//
//	PURPOSE:	Determine if action has completed.
//
// ----------------------------------------------------------------------- //

bool CAIActionGotoNodeOfType::IsActionComplete( CAI* pAI )
{
	// Goto is complete if state is complete.

	if( ( pAI->GetState() ) &&
		( pAI->GetState()->GetStateStatus() == kAIStateStatus_Complete ) )
	{
		return true;
	}

	// Goto is not complete.

	return false;
}

