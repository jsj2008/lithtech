// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionTaseredRecovery.cpp
//
// PURPOSE : 
//
// CREATED : 2/16/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIActionTaseredRecovery.h"
#include "AIStateUseSmartObject.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionTaseredRecovery, kAct_TaseredRecovery );


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionTaseredRecovery::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAIActionTaseredRecovery::CAIActionTaseredRecovery()
{
}

CAIActionTaseredRecovery::~CAIActionTaseredRecovery()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionTaseredRecovery::InitAction
//
//	PURPOSE:	Handle initializing the AIAction, setting the 
//				ActionRecord to use, as well as preconditions and effects.
//
// ----------------------------------------------------------------------- //

void CAIActionTaseredRecovery::InitAction( AIDB_ActionRecord* pActionRecord )
{
	super::InitAction( pActionRecord );

	// Set preconditions.
	// AI must already have reacted to being stunned by a taser before he can 
	// recover from the stun.

	m_wsWorldStatePreconditions.SetWSProp( kWSK_ReactedToWorldStateEvent, NULL, kWST_ENUM_AIWorldStateEvent, kWSE_Taser1Stunned );

	// Set effects.
	// AI reacted to damage.

	m_wsWorldStateEffects.SetWSProp( kWSK_ReactedToWorldStateEvent, NULL, kWST_ENUM_AIWorldStateEvent, kWSE_Damage );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionUseSmartObjectNode::ValidateContextPreconditions
//
//	PURPOSE:	Returns true if this action is valid given the AI and 
//				the goal WorldState
//
// ----------------------------------------------------------------------- //

bool CAIActionTaseredRecovery::ValidateContextPreconditions( CAI* pAI, CAIWorldState& wsWorldStateGoal, bool bIsPlanning )
{
	if ( !super::ValidateContextPreconditions( pAI, wsWorldStateGoal, bIsPlanning ) )
	{
		return false;
	}

	// If we have no smartobject, this action is invalid as it depends on 
	// the smartobject for configuration information.

	AIDB_SmartObjectRecord* pSmartObjectRecord = g_pAIDB->GetAISmartObjectRecord( m_pActionRecord->eSmartObjectID );
	if ( !pSmartObjectRecord )
	{
		return false;
	}

	// Action is valid!

	return true;
}
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionTaseredRecovery::ActivateAction
//
//	PURPOSE:	Activate action.
//
// ----------------------------------------------------------------------- //

void CAIActionTaseredRecovery::ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal )
{
	super::ActivateAction( pAI, wsWorldStateGoal );

	// The validity of the smartobject should be validated in the 
	// ContextPrecondition test.  It should never be NULL here.

	AIDB_SmartObjectRecord* pSmartObjectRecord = g_pAIDB->GetAISmartObjectRecord( m_pActionRecord->eSmartObjectID );
	AIASSERT( pSmartObjectRecord, pAI->GetHOBJECT(), "CAIActionTaseredRecovery::ActivateAction : Action has no smartobject.  The action should not get this far as the context validation should cause failure earlier." );

	// Play the recovery animations.

	pAI->SetState( kState_UseSmartObject );
	CAIStateUseSmartObject* pStateUseSmartObject = (CAIStateUseSmartObject*)( pAI->GetState() );
	pStateUseSmartObject->SetSmartObject( pSmartObjectRecord );
	pStateUseSmartObject->SetLooping( false );
	pStateUseSmartObject->SetProp( kAPG_DamageDir, pAI->GetAnimationContext()->GetCurrentProp( kAPG_DamageDir ) );

	// Disable tracking and facing

	pAI->GetAIBlackBoard()->SetBBFaceTarget( false );
	pAI->GetAIBlackBoard()->SetBBTargetTrackerFlags( kTrackerFlag_None );

	// Enable the player to steal the AIs weapon.
	
	pAI->GetAIWeaponMgr()->SpawnPickupItems();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionTaseredRecovery::DeactivateAction
//
//	PURPOSE:	Deactivate action.
//
// ----------------------------------------------------------------------- //

void CAIActionTaseredRecovery::DeactivateAction( CAI* pAI )
{
	super::DeactivateAction( pAI );

	// Disable stealing the AIs weapon.

	pAI->GetAIWeaponMgr()->DestroyDroppedWeapons();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionTaseredRecovery::IsActionComplete
//
//	PURPOSE:	Determine if action has completed.
//
// ----------------------------------------------------------------------- //

bool CAIActionTaseredRecovery::IsActionComplete( CAI* pAI )
{
	// Recoiling is complete when the animation finishes.

	if( ( pAI->GetState() ) &&
		( pAI->GetState()->GetStateStatus() == kAIStateStatus_Complete ) )
	{
		return true;
	}

	// Recoiling is not complete.

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionTaseredRecovery::ApplyContextEffect
//
//	PURPOSE:	Apply affects to the real game world.
//
// ----------------------------------------------------------------------- //

void CAIActionTaseredRecovery::ApplyContextEffect( CAI* pAI, CAIWorldState* pwsWorldStateCur, CAIWorldState* pwsWorldStateGoal )
{
	// Actually apply the planner effects, which is not the 
	// default behavior of an Action running in context.

	ApplyWSEffect( pAI, pwsWorldStateCur, pwsWorldStateGoal );
}
