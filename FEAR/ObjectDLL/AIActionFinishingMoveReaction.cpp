// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionFinishingMoveReaction.cpp
//
// PURPOSE : 
//
// CREATED : 3/17/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIActionFinishingMoveReaction.h"
#include "AI.h"
#include "AIWorldState.h"
#include "AIStateUseSmartObject.h"
#include "AnimationPropStrings.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionFinishingMoveReaction, kAct_FinishingMoveReaction );

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionFinishingMoveReaction::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAIActionFinishingMoveReaction::CAIActionFinishingMoveReaction()
{
}

CAIActionFinishingMoveReaction::~CAIActionFinishingMoveReaction()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionFinishingMoveReaction::InitAction
//
//	PURPOSE:	Handle initializing the AIAction, setting the 
//				ActionRecord to use, as well as preconditions and effects.
//
// ----------------------------------------------------------------------- //

void CAIActionFinishingMoveReaction::InitAction( AIDB_ActionRecord* pActionRecord )
{
	super::InitAction( pActionRecord );

	// Set effects.
	// Reacted to the player attempting a finishing move on us while we're defeated.

	m_wsWorldStateEffects.SetWSProp( kWSK_ReactedToWorldStateEvent, NULL, kWST_ENUM_AIWorldStateEvent, kWSE_FinishingMove );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionFinishingMoveReaction::ValidateContextPreconditions
//
//	PURPOSE:	Return true if real-time preconditions are valid.
//
// ----------------------------------------------------------------------- //

bool CAIActionFinishingMoveReaction::ValidateContextPreconditions( CAI* pAI, CAIWorldState& wsWorldStateGoal, bool bIsPlanning )
{
	if ( !super::ValidateContextPreconditions( pAI, wsWorldStateGoal, bIsPlanning ) )
	{
		return false;
	}

	// Fail if there is no smartobject (one is required, so assert if there
	// isn't one)

	AIDB_SmartObjectRecord* pSmartObjectRecord = g_pAIDB->GetAISmartObjectRecord( m_pActionRecord->eSmartObjectID );
	if ( !pSmartObjectRecord )
	{
		AIASSERT( 0, pAI->GetHOBJECT(), "CAIActionFinishingMoveReaction::ValidateContextPreconditions: Action does not define a smartobject; AI cannot respond to player finishing moves." );
		return false;
	}

	// Filter out if the player hasn't requested a finishing move.

	if ( !pAI->GetAIBlackBoard()->GetBBFinishingSyncAction() )
	{
		return false;
	}

	// Success!

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionFinishingMoveReaction::ActivateAction
//
//	PURPOSE:	Activate action.
//
// ----------------------------------------------------------------------- //

void CAIActionFinishingMoveReaction::ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal )
{
	super::ActivateAction( pAI, wsWorldStateGoal );

	AIDB_SmartObjectRecord* pSmartObjectRecord = g_pAIDB->GetAISmartObjectRecord( m_pActionRecord->eSmartObjectID );

	// Set state state.

	pAI->SetState( kState_UseSmartObject );
	CAIStateUseSmartObject* pStateUseSmartObject = (CAIStateUseSmartObject*)( pAI->GetState() );
	pStateUseSmartObject->SetSmartObject( pSmartObjectRecord );

	// Pull the proper action out of the SyncAction.

	HRECORD hSyncAction = pAI->GetAIBlackBoard()->GetBBFinishingSyncAction();
	EnumAnimProp eAction = AnimPropUtils::Enum(g_pModelsDB->GetString(hSyncAction, "AIAction"));

	AIASSERT( hSyncAction, pAI->GetHOBJECT(), "CAIActionFinishingMoveReaction::ActivateAction: SyncAction not set!" );
	AIASSERT( eAction != kAP_Invalid, pAI->GetHOBJECT(), "CAIActionFinishingMoveReaction::ActivateAction: AIAction is invalid!" );

	// And set the animation props.

	pStateUseSmartObject->SetProp( kAPG_Action, eAction );
	pStateUseSmartObject->SetProp( kAPG_Weapon, pAI->GetAIBlackBoard()->GetBBPrimaryWeaponProp() );

	// Tell the player to start their action now.

	CAutoMessage cClientMsg;
	cClientMsg.Writeuint8(MID_PLAYER_EVENT);
	cClientMsg.Writeuint8(kPESyncAction);
	cClientMsg.WriteDatabaseRecord(g_pLTDatabase, hSyncAction);
	cClientMsg.WriteObject(pAI->GetHOBJECT());
	g_pLTServer->SendToClient(cClientMsg.Read(), NULL, MESSAGE_GUARANTEED);

	// Clean up the blackboard.

	pAI->GetAIBlackBoard()->SetBBFinishingSyncAction( NULL );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionFinishingMoveReaction::IsActionComplete
//
//	PURPOSE:	Determine if action has completed.
//
// ----------------------------------------------------------------------- //

bool CAIActionFinishingMoveReaction::IsActionComplete( CAI* pAI )
{
	// Drawing is complete when state has completed.

	if( ( pAI->GetState() ) &&
		( pAI->GetState()->GetStateStatus() == kAIStateStatus_Complete ) )
	{
		return true;
	}

	// Drawing is not complete.

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionFinishingMoveReaction::CAIActionFinishingMoveReaction
//
//	PURPOSE:	Kill and force the AI into ragdoll.
//
// ----------------------------------------------------------------------- //

void CAIActionFinishingMoveReaction::ApplyContextEffect( CAI* pAI, CAIWorldState* pwsWorldStateCur, CAIWorldState* pwsWorldStateGoal )
{
	super::ApplyContextEffect( pAI, pwsWorldStateCur, pwsWorldStateGoal );

	DamageStruct damage;
	damage.hDamager			= pAI->GetHOBJECT();
	damage.fImpulseForce	= 0.0f;
	damage.SetPositionalInfo(pAI->GetPosition(), LTVector::GetIdentity());
	damage.eType			= DT_MELEE;
	damage.fDamage			= damage.kInfiniteDamage;
	damage.DoDamage(pAI->GetHOBJECT(), pAI->GetHOBJECT());
}

