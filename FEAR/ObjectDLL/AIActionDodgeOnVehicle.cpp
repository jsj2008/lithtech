// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionDodgeOnVehicle.cpp
//
// PURPOSE : AIActionDodgeOnVehicle class implementation
//
// CREATED : 01/08/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIActionDodgeOnVehicle.h"
#include "AI.h"
#include "AIState.h"
#include "AIStateAnimate.h"
#include "AIBlackBoard.h"
#include "AIUtils.h"
#include "AnimationContext.h"
#include "NodeTrackerContext.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionDodgeOnVehicle, kAct_DodgeOnVehicle );


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionDodgeOnVehicle::Con/destructor
//
//	PURPOSE:	Con/destructor
//
// ----------------------------------------------------------------------- //

CAIActionDodgeOnVehicle::CAIActionDodgeOnVehicle()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionDodgeOnVehicle::InitAction
//
//	PURPOSE:	Handle initializing the AIAction, setting the 
//				ActionRecord to use, as well as preconditions and effects.
//
// ----------------------------------------------------------------------- //

void CAIActionDodgeOnVehicle::InitAction( AIDB_ActionRecord* pActionRecord )
{
	super::InitAction( pActionRecord );

	// Set preconditions.

	// Set effects.
	// Target is no longer aiming at me.

	m_wsWorldStateEffects.SetWSProp( kWSK_TargetIsAimingAtMe, NULL, kWST_bool, false );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionDodgeOnVehicle::ValidateContextPreconditions
//
//	PURPOSE:	Return true if real-time preconditions are valid.
//
// ----------------------------------------------------------------------- //

bool CAIActionDodgeOnVehicle::ValidateContextPreconditions( CAI* pAI, CAIWorldState& wsWorldStateGoal, bool bIsPlanning )
{
	// Action is only valid if AI is riding a Vehicle.

	SAIWORLDSTATE_PROP* pProp = pAI->GetAIWorldState()->GetWSProp( kWSK_RidingVehicle, pAI->m_hObject );
	if( ( !pProp ) ||
		( pProp->eAnimPropWSValue == kAP_None ) )
	{
		return false;
	}

	// Only dodge if we are armed.

	pProp = pAI->GetAIWorldState()->GetWSProp( kWSK_WeaponArmed, pAI->m_hObject );
	if( pProp && !pProp->bWSValue )
	{
		return false;
	}

	// Only dodge if the AI is not trying to dodge a blow

	CAIWMFact queryFact;
	queryFact.SetFactType( kFact_Desire );
	queryFact.SetDesireType( kDesire_CounterMelee );
	CAIWMFact* pFact = pAI->GetAIWorkingMemory()->FindWMFact( queryFact );
	if ( pFact && ( 0.0f != pFact->GetConfidence( CAIWMFact::kFactMask_DesireType ) ) )
	{
		return false;
	}

	// Dodge animation does not exist for this vehicle.

	CAnimationProps AnimProps;
	AnimProps.Set( kAPG_Posture, kAP_POS_Stand );
	AnimProps.Set( kAPG_WeaponPosition, kAP_WPOS_Up );
	AnimProps.Set( kAPG_Weapon, pAI->GetAIBlackBoard()->GetBBPrimaryWeaponProp() );
	AnimProps.Set( kAPG_MovementDir, kAP_MDIR_Forward );
	AnimProps.Set( kAPG_Activity, pProp->eAnimPropWSValue );
	AnimProps.Set( kAPG_Action, kAP_ACT_Dodge );

	uint32 nAnimations = pAI->GetAnimationContext()->CountAnimations( AnimProps );
	if( !nAnimations )
	{
		return false;
	}

	// Do not dodge.

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionDodgeOnVehicle::ActivateAction
//
//	PURPOSE:	Activate action.
//
// ----------------------------------------------------------------------- //

void CAIActionDodgeOnVehicle::ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal )
{
	super::ActivateAction( pAI, wsWorldStateGoal );

	// Get vehicle animProp.

	SAIWORLDSTATE_PROP* pProp = pAI->GetAIWorldState()->GetWSProp( kWSK_RidingVehicle, pAI->m_hObject );
	if( !pProp )
	{
		return;
	}
	EnumAnimProp eVehicle = pProp->eAnimPropWSValue;

	// Get attack direction.

	EnumAnimProp eDir = pAI->GetAnimationContext()->GetCurrentProp( kAPG_MovementDir );
	switch( eDir )
	{
		case kAP_MDIR_Forward: 
		case kAP_MDIR_Backward: 
		case kAP_MDIR_Right: 
		case kAP_MDIR_Left: 
			break;

		default:
			eDir = kAP_MDIR_Forward;
	}

	// Set animate state.
	
	pAI->SetState( kState_Animate );

	// Set dodge animation.

	CAnimationProps	animProps;
	animProps.Set( kAPG_Posture, kAP_POS_Stand );
	animProps.Set( kAPG_Weapon, pAI->GetAIBlackBoard()->GetBBPrimaryWeaponProp() );
	animProps.Set( kAPG_WeaponPosition, kAP_WPOS_Up );
	animProps.Set( kAPG_MovementDir, eDir );
	animProps.Set( kAPG_Action, kAP_ACT_Dodge );
	animProps.Set( kAPG_Activity, eVehicle );

	CAIStateAnimate* pStateAnimate = (CAIStateAnimate*)( pAI->GetState() );
	pStateAnimate->SetAnimation( animProps, !LOOP );

	// Torso tracking.

	pAI->GetAIBlackBoard()->SetBBTargetTrackerFlags( kTrackerFlag_Arm );
	pAI->GetAIBlackBoard()->SetBBFaceTarget( false );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionDodgeOnVehicle::IsActionComplete
//
//	PURPOSE:	Determine if action has completed.
//
// ----------------------------------------------------------------------- //

bool CAIActionDodgeOnVehicle::IsActionComplete( CAI* pAI )
{
	// Dodging is complete when the animation finishes.

	if( ( pAI->GetState() ) &&
		( pAI->GetState()->GetStateStatus() == kAIStateStatus_Complete ) )
	{
		return true;
	}

	// Dodging is not complete.

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionDodgeOnVehicle::ApplyContextEffect
//
//	PURPOSE:	Apply affects to the real game world.
//
// ----------------------------------------------------------------------- //

void CAIActionDodgeOnVehicle::ApplyContextEffect( CAI* pAI, CAIWorldState* pwsWorldStateCur, CAIWorldState* pwsWorldStateGoal )
{
	// Actually apply the planner effects, which is not the 
	// default behavior of an Action running in context.

	ApplyWSEffect( pAI, pwsWorldStateCur, pwsWorldStateGoal );
}
