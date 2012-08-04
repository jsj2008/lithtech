// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionDismountNode.cpp
//
// PURPOSE : AIActionDismountNode class implementation
//
// CREATED : 04/22/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIActionDismountNode.h"
#include "AI.h"
#include "AIDB.h"
#include "AINode.h"
#include "AIStateUseSmartObject.h"
#include "AIBlackBoard.h"
#include "AIWorkingMemory.h"
#include "NodeTrackerContext.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionDismountNode, kAct_DismountNode );


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionDismountNode::Con/destructor
//
//	PURPOSE:	Con/destructor
//
// ----------------------------------------------------------------------- //

CAIActionDismountNode::CAIActionDismountNode()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionDismountNode::InitAction
//
//	PURPOSE:	Handle initializing the AIAction, setting the 
//				ActionRecord to use, as well as preconditions and effects.
//
// ----------------------------------------------------------------------- //

void CAIActionDismountNode::InitAction( AIDB_ActionRecord* pActionRecord )
{
	super::InitAction( pActionRecord );

	// Remove super classes' precondition.

	m_wsWorldStatePreconditions.ClearWSProp( kWSK_AtNode, NULL );

	// Set effects.
	// AI has dismounted the node.

	m_wsWorldStateEffects.SetWSProp( kWSK_MountedObject, NULL, kWST_HOBJECT, 0 );

	// Remove super classes' effect.

	m_wsWorldStateEffects.ClearWSProp( kWSK_UsingObject, NULL );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionDismountNode::ValidateWSPreconditions
//
//	PURPOSE:	Return true if this action's preconditions are met in 
//              plan's current world state.
//
// ----------------------------------------------------------------------- //

bool CAIActionDismountNode::ValidateWSPreconditions( CAI* pAI, CAIWorldState& wsWorldStateCur, CAIWorldState& wsWorldStateGoal, ENUM_AIWORLDSTATE_PROP_KEY* pFailedWSK )
{
	if( !super::ValidateWSPreconditions( pAI, wsWorldStateCur, wsWorldStateGoal, pFailedWSK ))
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

	// AI must be mounted.  This is not a precondition, as a setting it as a 
	// precondition would cause the AI to attempt to formulate plans (ie we 
	// want to dismount, so go to a node and mount it so that you can dismount).

	SAIWORLDSTATE_PROP* pMountedProp = pAI->GetAIWorldState()->GetWSProp( kWSK_MountedObject, pAI->m_hObject );
	if (!pMountedProp || !IsAINode(pMountedProp->hWSValue) )
	{
		if( pFailedWSK )
		{
			*pFailedWSK = pMountedProp->eWSKey;
		}
		return false;
	}

	// Preconditions are valid.

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionDismountNode::ActivateAction
//
//	PURPOSE:	Activate action.
//
// ----------------------------------------------------------------------- //

void CAIActionDismountNode::ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal )
{
	// Intentionally do NOT call super::ActivateAction().
	// This action is only activated when the AI is already at a node.
	// Get the SmartObject from the node the AI is using, rather
	// than from a variable.
	//
	// Be sure to call down to the base class however to insure that any 
	// awareness modifications are applied.
	CAIActionAbstract::ActivateAction( pAI, wsWorldStateGoal );

	// Bail if we are not at a node.

	SAIWORLDSTATE_PROP* pProp = pAI->GetAIWorldState()->GetWSProp( kWSK_AtNode, pAI->m_hObject );
	if( !pProp )
	{
		return;
	}

	// Bail if node does not exist.

	AINodeSmartObject* pNodeSmartObject = (AINodeSmartObject*)g_pLTServer->HandleToObject( pProp->hWSValue );
	if( !pNodeSmartObject )
	{
		return;
	}

	// Bail if the smart object command for the node does not exist.
	// Consider the node immediately used.

	AIDB_SmartObjectRecord* pSmartObject = pNodeSmartObject->GetSmartObject();
	if( ( !pSmartObject ) ||
		(  pSmartObject->eNodeType == kNode_InvalidType ) )
	{
		pAI->GetAIWorldState()->SetWSProp( kWSK_UsingObject, pAI->m_hObject, kWST_HOBJECT, pProp->hWSValue );
		return;
	}

	// Set UseSmartObject state.

	pAI->SetState( kState_UseSmartObject );
	CAIStateUseSmartObject* pStateUseSmartObject = (CAIStateUseSmartObject*)( pAI->GetState() );

	// Set the node to use.

	pStateUseSmartObject->SetNode( pNodeSmartObject );

	// Set the smart object command for the node.

	pStateUseSmartObject->SetSmartObject( pSmartObject );

	// Set the Dismount Action animProp.

	pStateUseSmartObject->SetProp( kAPG_Action, GetDismountAction() );
	pStateUseSmartObject->SetLooping( false );


	// Torso tracking.

	if( pAI->GetAIBlackBoard()->GetBBAwareness() == kAware_Alert )
	{
		// Aim ranged weapon at target.

		if( AIWeaponUtils::HasWeaponType( pAI, kAIWeaponType_Ranged, !AIWEAP_CHECK_HOLSTER ) )
		{
			pAI->GetAIBlackBoard()->SetBBTargetTrackerFlags( kTrackerFlag_AimAt );
		}

		// Look at target.

		else {
			pAI->GetAIBlackBoard()->SetBBTargetTrackerFlags( kTrackerFlag_LookAt );
		}
	}
	else {
		pAI->GetAIBlackBoard()->SetBBTargetTrackerFlags( kTrackerFlag_None );
	}
	pAI->GetAIBlackBoard()->SetBBFaceTarget( false );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionDismountNode::ValidateAction
//
//	PURPOSE:	Return true if action is still valid.
//
// ----------------------------------------------------------------------- //

bool CAIActionDismountNode::ValidateAction( CAI* pAI )
{
	// Intentionally do NOT call super::ValidateAction.
	// We don't care if the node is invalid.

	if( !CAIActionAbstract::ValidateAction( pAI ) )
	{
		return false;
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionDismountNode::ApplyContextEffect
//
//	PURPOSE:	Apply affects to the real game world.
//
// ----------------------------------------------------------------------- //

void CAIActionDismountNode::ApplyContextEffect( CAI* pAI, CAIWorldState* pwsWorldStateCur, CAIWorldState* pwsWorldStateGoal )
{
	// Sanity check.

	if( !pAI )
	{
		return;
	}

	// Actually apply the planner effects, which is not the 
	// default behavior of an Action running in context.

	ApplyWSEffect( pAI, pwsWorldStateCur, pwsWorldStateGoal );
}

