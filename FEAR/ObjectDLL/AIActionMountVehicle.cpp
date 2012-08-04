// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionMountVehicle.cpp
//
// PURPOSE : AIActionMountVehicle class implementation
//
// CREATED : 12/23/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIActionMountVehicle.h"
#include "AI.h"
#include "AIDB.h"
#include "AINode.h"
#include "AIStateUseSmartObject.h"
#include "AIBlackBoard.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionMountVehicle, kAct_MountVehicle );


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionMountVehicle::ValidateContextPreconditions
//
//	PURPOSE:	Return true if real-time preconditions are valid.
//
// ----------------------------------------------------------------------- //

bool CAIActionMountVehicle::ValidateContextPreconditions( CAI* pAI, CAIWorldState& wsWorldStateGoal, bool bIsPlanning )
{
	if( !super::ValidateContextPreconditions( pAI, wsWorldStateGoal, bIsPlanning ) )
	{
		return false;
	}
	
	// Action is only valid if AI is at a Vehicle node.

	SAIWORLDSTATE_PROP* pProp = wsWorldStateGoal.GetWSProp( kWSK_UsingObject, pAI->m_hObject );
	if( !( pProp && pProp->hWSValue ) )
	{
		return false;
	}

	AINodeVehicle* pVehicleNode = AINodeVehicle::DynamicCast( pProp->hWSValue );
	if( !pVehicleNode )
	{
		return false;
	}

	// Action is valid.

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionMountVehicle::ActivateAction
//
//	PURPOSE:	Activate action.
//
// ----------------------------------------------------------------------- //

void CAIActionMountVehicle::ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal )
{
	super::ActivateAction( pAI, wsWorldStateGoal );

	// Sanity check.

	if( pAI->GetState()->GetStateClassType() != kState_UseSmartObject )
	{
		return;
	}

	// Set the Mount Action animProp.

	CAIStateUseSmartObject* pStateUseSmartObject = (CAIStateUseSmartObject*)( pAI->GetState() );
	pStateUseSmartObject->SetProp( kAPG_Action, kAP_ACT_Mount );
	pStateUseSmartObject->SetLooping( false );

	// Set the KeyframeToRigidBody object if it exists.

	SAIWORLDSTATE_PROP* pProp = wsWorldStateGoal.GetWSProp( kWSK_UsingObject, pAI->m_hObject );
	if( pProp )
	{
		// Bail if node does not exist.

		AINodeVehicle* pNodeVehicle = AINodeVehicle::DynamicCast( pProp->hWSValue );
		if( !pNodeVehicle )
		{
			return;
		}

		// Record the node's KeyframeToRigidBody object.

		HOBJECT hKeyframeToRigidBody = pNodeVehicle->GetVehicleKeyframeToRigidBody();
		if( hKeyframeToRigidBody )
		{
			pAI->GetAIBlackBoard()->SetBBVehicleKeyframeToRigidBody( hKeyframeToRigidBody );
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionMountVehicle::ApplyContextEffect
//
//	PURPOSE:	Apply affects to the real game world.
//
// ----------------------------------------------------------------------- //

void CAIActionMountVehicle::ApplyContextEffect( CAI* pAI, CAIWorldState* pwsWorldStateCur, CAIWorldState* pwsWorldStateGoal )
{
	// Sanity check.

	if( !( pAI && pwsWorldStateCur && pwsWorldStateGoal ) )
	{
		return;
	}

	// Bail if WorldState has no knowledge of AINode being used.

	SAIWORLDSTATE_PROP* pProp = pwsWorldStateGoal->GetWSProp( kWSK_UsingObject, pAI->m_hObject );
	if( !( pProp && pProp->hWSValue ) )
	{
		return;
	}

	// Bail if AINode is not an AINodeVehicle.

	AINodeVehicle* pNodeVehicle = AINodeVehicle::DynamicCast( pProp->hWSValue );
	if( !pNodeVehicle )
	{
		return;
	}

	// Bail if vehicle object cannot be found.

	if( !pNodeVehicle->HasObject() )
	{
		return;
	}

	// Attach the AI to the vehicle.

	HOBJECT hVehicle = pNodeVehicle->GetObject();
	std::string strCmd = "ATTACH ";
	strCmd += pAI->GetName();
	g_pCmdMgr->QueueMessage( pAI, g_pLTServer->HandleToObject(hVehicle), strCmd.c_str() );

	// Record the vehicle the AI is attached to on the Blackboard.

	pAI->GetAIBlackBoard()->SetBBAttachedTo( hVehicle );

	// Record the type of vehicle being ridden in the WorldState.

	AIDB_SmartObjectRecord* pSmartObject = pNodeVehicle->GetSmartObject();
	if( pSmartObject )
	{
		EnumAnimProp eVehicle = pSmartObject->Props.Get( kAPG_Activity );
		pAI->GetAIWorldState()->SetWSProp( kWSK_RidingVehicle, pAI->m_hObject, kWST_EnumAnimProp, eVehicle );
	}

	// Run the PostActivate command on the node.

	super::ApplyContextEffect( pAI, pwsWorldStateCur, pwsWorldStateGoal );
}
