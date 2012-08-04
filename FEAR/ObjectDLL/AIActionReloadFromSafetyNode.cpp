// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionReloadFromSafetyNode.cpp
//
// PURPOSE : This action handles the desire to reload by moving to an 
//			AINodeSafety and performing the reload there.
//
// NOTE	   : This class assumes the weapon to be reloaded is the AIs ranged 
//			weapon.  AIActionReload makes this same assumption; if melee 
//			weapons start requiring reloading, this assumption may change.
//
// CREATED : 1/31/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIActionReloadFromSafetyNode.h"
#include "AINode.h"
#include "AIStateAnimate.h"
#include "AIDB.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionReloadFromSafetyNode, kAct_ReloadFromSafetyNode );

static HOBJECT GetAtNode( CAI* pAI, CAIWorldState& WorldState )
{
	// Bail if we are not at a node.

	SAIWORLDSTATE_PROP* pProp = WorldState.GetWSProp( kWSK_AtNode, pAI->m_hObject );
	if( !pProp )
	{
		return NULL;
	}

	return pProp->hWSValue;
}

static AIDB_SmartObjectRecord* GetSmartObjectFromNode( CAI* pAI, CAIWorldState& WorldState )
{
	// Bail if we are not at a node.

	SAIWORLDSTATE_PROP* pProp = WorldState.GetWSProp( kWSK_AtNode, pAI->m_hObject );
	if( !pProp )
	{
		return NULL;
	}

	// Bail if node does not exist.

	AINodeSmartObject* pNodeSmartObject = (AINodeSmartObject*)g_pLTServer->HandleToObject( pProp->hWSValue );
	if( !pNodeSmartObject )
	{
		return NULL;
	}
	
	// Bail if the smart object command for the node does not exist.

	return pNodeSmartObject->GetSmartObject();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionReloadFromSafetyNode::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAIActionReloadFromSafetyNode::CAIActionReloadFromSafetyNode()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionReloadFromSafetyNode::InitAction
//
//	PURPOSE:	Handle initializing the AIAction, setting the 
//				ActionRecord to use, as well as preconditions and effects.
//
// ----------------------------------------------------------------------- //

void CAIActionReloadFromSafetyNode::InitAction( AIDB_ActionRecord* pActionRecord )
{
	super::InitAction( pActionRecord );

	// Set preconditions.
	// Weapon must be armed.
	// AI must be at a safety node.

//	m_wsWorldStatePreconditions.SetWSProp( kWSK_WeaponArmed, NULL, kWST_bool, true );
	m_wsWorldStatePreconditions.SetWSProp( kWSK_AtNodeType, NULL, kWST_EnumAINodeType, kNode_Safety );

	// Set effects.
	// Weapon is loaded.

	m_wsWorldStateEffects.SetWSProp( kWSK_WeaponLoaded, NULL, kWST_bool, true );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionReloadFromSafetyNode::ValidateContextPreconditions
//
//	PURPOSE:	Return true if real-time preconditions are valid.
//
// ----------------------------------------------------------------------- //

bool CAIActionReloadFromSafetyNode::ValidateContextPreconditions( CAI* pAI, CAIWorldState& wsWorldStateGoal, bool bIsPlanning )
{
	if ( !super::ValidateContextPreconditions( pAI, wsWorldStateGoal, bIsPlanning ) )
	{
		return false;
	}

	// Verify the AI has no ammo right now.  This prevents this action from 
	// being used in the middle of a plan, but prevents the redundant evaluation 
	// of this action.

	SAIWORLDSTATE_PROP* pProp = pAI->GetAIWorldState()->GetWSProp( kWSK_WeaponLoaded, pAI->GetHOBJECT() );
	if ( !pProp || pProp->bWSValue )
	{
		return false;
	}
	
	// No such weapon type.

	AIDB_AIWeaponRecord* pWeaponRecord = g_pAIDB->GetAIWeaponRecord(pAI->GetAIBlackBoard()->GetBBCurrentAIWeaponRecordID());
	if ( !pWeaponRecord )
	{
		return false;
	}

	// No ammo of this type.
        
	if ( !AIWeaponUtils::HasAmmo( pAI, pWeaponRecord->eAIWeaponType, bIsPlanning ) )
	{
		return false;
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionAbstract::ValidateWSPreconditions
//
//	PURPOSE:	Return true if this action's preconditions are met in 
//              plan's current world state.
//
// ----------------------------------------------------------------------- //

bool CAIActionReloadFromSafetyNode::ValidateWSPreconditions( CAI* pAI, CAIWorldState& wsWorldStateCur, CAIWorldState& wsWorldStateGoal, ENUM_AIWORLDSTATE_PROP_KEY* pFailedWSK )
{
	if ( !super::ValidateWSPreconditions( pAI, wsWorldStateCur, wsWorldStateGoal, pFailedWSK ) )
	{
		return false;
	}

	// No SmartObject at the node the AI is going to.  This is a level 
	// designer error, as this node will never be used.  This test prevents 
	// failure inside of ActivateAction -- the error itself is reported inside 
	// the AINodes init.

	AIDB_SmartObjectRecord* pSmartObject = GetSmartObjectFromNode( pAI, wsWorldStateCur );
	if ( !pSmartObject )
	{
		return false;
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionReloadFromSafetyNode::ActivateAction
//
//	PURPOSE:	Activate action.
//
// ----------------------------------------------------------------------- //

void CAIActionReloadFromSafetyNode::ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal )
{
	super::ActivateAction( pAI, wsWorldStateGoal );

	// Bail if the smart object command for the node does not exist.

	AIDB_SmartObjectRecord* pSmartObject = GetSmartObjectFromNode( pAI, *pAI->GetAIWorldState() );
	if ( !pSmartObject )
	{
		AIASSERT( 0, pAI->GetHOBJECT(), "CAIActionReloadFromSafetyNode::ActivateAction: AI at node without a smartobject.  This node be filtered out (and reported as an error) during the ContextPrecondition tests." );
		return;
	}

	// Override the action, setting it to reload and clear any activity.

	CAnimationProps animProps = pSmartObject->Props;
	animProps.Set( kAPG_Action, kAP_ACT_Reload );
	animProps.Set( kAPG_Activity, kAP_None );

	// Reload our ranged weapon.

	pAI->SetCurrentWeapon( kAIWeaponType_Ranged );

	// Set animate state.
	
	pAI->SetState( kState_Animate );

	// Set reload animation.

	CAIStateAnimate* pStateAnimate = (CAIStateAnimate*)( pAI->GetState() );
	pStateAnimate->SetAnimation( animProps, !LOOP );

	// Torso tracking.

	pAI->GetAIBlackBoard()->SetBBTargetTrackerFlags( kTrackerFlag_AimAt );
	pAI->GetAIBlackBoard()->SetBBFaceTarget( true );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionReloadFromSafetyNode::IsActionComplete
//
//	PURPOSE:	Determine if action has completed.
//
// ----------------------------------------------------------------------- //

bool CAIActionReloadFromSafetyNode::IsActionComplete( CAI* pAI )
{
	// Reloading is complete when the animation finishes.

	if( pAI &&
		( pAI->GetState() ) 
		&& ( pAI->GetState()->GetStateStatus() == kAIStateStatus_Complete ) )
	{
		return true;
	}

	// Reloading is not complete.

	return false;
}

