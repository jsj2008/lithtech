// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionCounterMeleeBlock.cpp
//
// PURPOSE : 
//
// CREATED : 9/14/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIActionCounterMeleeBlock.h"
#include "AIStateAnimate.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionCounterMeleeBlock, kAct_CounterMeleeBlock );

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionCounterMeleeBlock::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAIActionCounterMeleeBlock::CAIActionCounterMeleeBlock()
{
}

CAIActionCounterMeleeBlock::~CAIActionCounterMeleeBlock()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionCounterMeleeBlock::InitAction
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

void CAIActionCounterMeleeBlock::InitAction( AIDB_ActionRecord* pActionRecord )
{
	super::InitAction( pActionRecord );

	// Set preconditions.

	// Set effects.
	// AI reacted to the incoming attack.

	m_wsWorldStateEffects.SetWSProp( kWSK_ReactedToWorldStateEvent, NULL, kWST_ENUM_AIWorldStateEvent, kWSE_IncomingMeleeAttack );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionCounterMeleeBlock::ValidateContextPreconditions
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

bool CAIActionCounterMeleeBlock::ValidateContextPreconditions( CAI* pAI, CAIWorldState& wsWorldStateGoal, bool bIsPlanning )
{
	// Fail if AI is attached to something.
	
	HOBJECT hAttached = pAI->GetAIBlackBoard()->GetBBAttachedTo();
	if( hAttached )
	{
		return false;
	}

	// Bail if the Action's SmartObject record does not exist.

	AIDB_SmartObjectRecord* pSmartObjectRecord = g_pAIDB->GetAISmartObjectRecord( m_pActionRecord->eSmartObjectID );
	if( !pSmartObjectRecord )
	{
		return false;
	}

	// Only block if we are armed.

	SAIWORLDSTATE_PROP* pProp = pAI->GetAIWorldState()->GetWSProp( kWSK_WeaponArmed, pAI->m_hObject );
	if( pProp && !pProp->bWSValue )
	{
		return false;
	}


	// Do not block if the AI doesn't know about an incoming melee attack

	CAIWMFact queryFact;
	queryFact.SetFactType( kFact_Desire );
	queryFact.SetDesireType( kDesire_CounterMelee );
	CAIWMFact* pFact = pAI->GetAIWorkingMemory()->FindWMFact( queryFact );
	if ( !pFact )
	{
		return false;
	}

	// Do not block if the targeted object is not the melee attacker

	if ( pAI->GetAIBlackBoard()->GetBBTargetObject() != pFact->GetSourceObject() )
	{
		return false;
	}

	// Do not block if the target is not exposed to blocking

	CCharacter* pChar = CCharacter::DynamicCast( pAI->GetAIBlackBoard()->GetBBTargetObject() );
	if ( !pChar || !pChar->GetBlockWindowOpen() )
	{
		return false;
	}

	// Do not block if the AI can't see its enemy (the enemy must be in front of the AI).

	LTVector vToAttackerNorm = ( pAI->GetAIBlackBoard()->GetBBTargetPosition() - pAI->GetPosition() ).GetUnit();

	const float kflNormalFOV = 0.0f;
	if ( !pAI->IsInsideFOV( kflNormalFOV, vToAttackerNorm ) )
	{
		return false;
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionCounterMeleeBlock::ActivateAction
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

void CAIActionCounterMeleeBlock::ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal )
{
	super::ActivateAction( pAI, wsWorldStateGoal );

	// Bail if the Action's SmartObject record does not exist.

	AIDB_SmartObjectRecord* pSmartObjectRecord = g_pAIDB->GetAISmartObjectRecord( m_pActionRecord->eSmartObjectID );
	if( !pSmartObjectRecord )
	{
		return;
	}

	// Set animate state.
	
	pAI->SetState( kState_Animate );

	// Set block animation.

	CAIStateAnimate* pStateAnimate = (CAIStateAnimate*)( pAI->GetState() );
	CAnimationProps props = pSmartObjectRecord->Props;
	props.Set( kAPG_Weapon, pAI->GetAIBlackBoard()->GetBBPrimaryWeaponProp() );
	pStateAnimate->SetAnimation( props, !LOOP );

	// Torso tracking.

	pAI->GetAIBlackBoard()->SetBBTargetTrackerFlags( kTrackerFlag_AimAt );
	pAI->GetAIBlackBoard()->SetBBFaceTarget( true );

	// Inform the client.

	CAutoMessage cClientMsg;
	cClientMsg.Writeuint8( MID_SFX_MESSAGE );
	cClientMsg.Writeuint8( SFX_CHARACTER_ID );
	cClientMsg.WriteObject( pAI->m_hObject );
	cClientMsg.Writeuint8( CFX_MELEE_CONTROLLER_MSG );
	cClientMsg.Writeuint8( MELEE_FORCE_BLOCKING_MSG );
	cClientMsg.Writebool( true );
	g_pLTServer->SendToClient( cClientMsg.Read(), NULL, MESSAGE_GUARANTEED );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionCounterMeleeBlock::DeactivateAction
//
//	PURPOSE:	Deactivate action.  If the block was successful, add a 
//				desire to perform a counter attack.  If it was 
//				unsuccessful, do not attempt a counter attack.  To handle
//				probability/randomization, a non attack animation
//
// ----------------------------------------------------------------------- //

void CAIActionCounterMeleeBlock::DeactivateAction( CAI* pAI )
{
	super::DeactivateAction( pAI );

	// Determine if the AI successfully blocked and remove the fact.
	
	bool bBlocked = false;

	CAIWMFact queryFact;
	queryFact.SetFactType( kFact_Knowledge );
	queryFact.SetKnowledgeType( kKnowledge_MeleeBlockedAttacker );

	CAIWMFact* pFact = pAI->GetAIWorkingMemory()->FindWMFact( queryFact );
	if ( pFact )
	{
		pAI->GetAIWorkingMemory()->ClearWMFact( queryFact );
		bBlocked = true;
	}

	// Add a desire to 'finish' the attack with an appropriate animation, if 
	// the action is complete.  The action may not be finished if the action 
	// was interrupted.

	if ( IsActionComplete( pAI ) )
	{
		ENUM_AIWMKNOWLEDGE_TYPE eFinishTypeKnowledge = bBlocked ? kKnowledge_MeleeBlockSuccess : kKnowledge_MeleeBlockFailure;

		CAIWMFact queryFact;
		queryFact.SetFactType( kFact_Knowledge );
		queryFact.SetKnowledgeType( eFinishTypeKnowledge );
		CAIWMFact* pBlockedFinishFact = pAI->GetAIWorkingMemory()->FindWMFact( queryFact );

		if ( !pBlockedFinishFact ) 
		{
			pBlockedFinishFact = pAI->GetAIWorkingMemory()->CreateWMFact( kFact_Knowledge );
			pBlockedFinishFact->SetKnowledgeType( eFinishTypeKnowledge );
		}

		if ( pBlockedFinishFact )
		{
			pBlockedFinishFact->SetTime( g_pLTServer->GetTime() );
		}
	}

	// Inform the client.

	CAutoMessage cClientMsg;
	cClientMsg.Writeuint8( MID_SFX_MESSAGE );
	cClientMsg.Writeuint8( SFX_CHARACTER_ID );
	cClientMsg.WriteObject( pAI->m_hObject );
	cClientMsg.Writeuint8( CFX_MELEE_CONTROLLER_MSG );
	cClientMsg.Writeuint8( MELEE_FORCE_BLOCKING_MSG );
	cClientMsg.Writebool( false );
	g_pLTServer->SendToClient( cClientMsg.Read(), NULL, MESSAGE_GUARANTEED );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionCounterMeleeBlock::IsActionComplete
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

bool CAIActionCounterMeleeBlock::IsActionComplete( CAI* pAI )
{
	// Dismount is complete when the animation finishes.

	if( ( pAI->GetState() ) &&
		( pAI->GetState()->GetStateStatus() == kAIStateStatus_Complete ) )
	{
		return true;
	}

	// Dismount is not complete.

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionCounterMeleeBlock::ApplyContextEffect
//
//	PURPOSE:	Apply affects to the real game world.
//
// ----------------------------------------------------------------------- //

void CAIActionCounterMeleeBlock::ApplyContextEffect( CAI* pAI, CAIWorldState* pwsWorldStateCur, CAIWorldState* pwsWorldStateGoal )
{
	// Actually apply the planner effects, which is not the 
	// default behavior of an Action running in context.

	ApplyWSEffect( pAI, pwsWorldStateCur, pwsWorldStateGoal );
}
