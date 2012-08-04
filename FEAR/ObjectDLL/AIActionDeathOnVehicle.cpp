// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionDeathOnVehicle.cpp
//
// PURPOSE : AIActionDeathOnVehicle class implementation
//
// CREATED : 04/15/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIActionDeathOnVehicle.h"
#include "AI.h"
#include "AIStateAnimate.h"
#include "AIBlackBoard.h"
#include "AIUtils.h"
#include "AnimationContext.h"
#include "NodeTrackerContext.h"
#include "KeyframeToRigidBody.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionDeathOnVehicle, kAct_DeathOnVehicle );


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionDeathOnVehicle::Con/destructor
//
//	PURPOSE:	Con/destructor
//
// ----------------------------------------------------------------------- //

CAIActionDeathOnVehicle::CAIActionDeathOnVehicle()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionDeathOnVehicle::InitAction
//
//	PURPOSE:	Handle initializing the AIAction, setting the 
//				ActionRecord to use, as well as preconditions and effects.
//
// ----------------------------------------------------------------------- //

void CAIActionDeathOnVehicle::InitAction( AIDB_ActionRecord* pActionRecord )
{
	super::InitAction( pActionRecord );

	// Set preconditions.

	// Set effects.
	// AI reacted to damage.

	m_wsWorldStateEffects.SetWSProp( kWSK_ReactedToWorldStateEvent, NULL, kWST_ENUM_AIWorldStateEvent, kWSE_Damage );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionDeathOnVehicle::ValidateContextPreconditions
//
//	PURPOSE:	Return true if real-time preconditions are valid.
//
// ----------------------------------------------------------------------- //

bool CAIActionDeathOnVehicle::ValidateContextPreconditions( CAI* pAI, CAIWorldState& wsWorldStateGoal, bool bIsPlanning )
{
	// Action is only valid if AI is riding a Vehicle.

	SAIWORLDSTATE_PROP* pProp = pAI->GetAIWorldState()->GetWSProp( kWSK_RidingVehicle, pAI->m_hObject );
	if( ( !pProp ) ||
		( pProp->eAnimPropWSValue == kAP_None ) )
	{
		return false;
	}
	EnumAnimProp eVehicle = pProp->eAnimPropWSValue;

	// Action is only valid if AI is dead!

	if( !pAI->GetDestructible()->IsDead() )
	{
		return false;
	}

	// Death animation does not exist.

	CAnimationProps	animProps;
	GetDeathAnimProps( pAI, &animProps, eVehicle );
	uint32 cAnimations = pAI->GetAnimationContext()->CountAnimations( animProps );
	if( 0 == cAnimations )
	{
		return false;
	}

	// Do not die.

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionDeathOnVehicle::ActivateAction
//
//	PURPOSE:	Activate action.
//
// ----------------------------------------------------------------------- //

void CAIActionDeathOnVehicle::ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal )
{
	super::ActivateAction( pAI, wsWorldStateGoal );

	// Get vehicle animProp.

	SAIWORLDSTATE_PROP* pProp = pAI->GetAIWorldState()->GetWSProp( kWSK_RidingVehicle, pAI->m_hObject );
	if( !pProp )
	{
		return;
	}
	EnumAnimProp eVehicle = pProp->eAnimPropWSValue;

	// Set animate state.
	
	pAI->SetState( kState_Animate );

	// Set death animation.

	CAnimationProps	animProps;
	GetDeathAnimProps( pAI, &animProps, eVehicle );

	CAIStateAnimate* pStateAnimate = (CAIStateAnimate*)( pAI->GetState() );
	pStateAnimate->SetAnimation( animProps, !LOOP );

	// Circumvent the normal CCharacter code path for handling 
	// deaths, and creating Body objects.

	pAI->GetAIBlackBoard()->SetBBAIHandlingDeath( true );

	// Torso tracking.

	pAI->GetAIBlackBoard()->SetBBTargetTrackerFlags( kTrackerFlag_None );
	pAI->GetAIBlackBoard()->SetBBFaceTarget( false );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionDeathOnVehicle::GetDeathAnimProps
//
//	PURPOSE:	Get the death anim props.
//
// ----------------------------------------------------------------------- //

void CAIActionDeathOnVehicle::GetDeathAnimProps( CAI* pAI, CAnimationProps*	pAnimProps, EnumAnimProp eVehicle )
{
	// Sanity check.

	if( !( pAI && pAnimProps ) )
	{
		return;
	}

	pAnimProps->Set( kAPG_Posture, kAP_POS_Stand );
	pAnimProps->Set( kAPG_Weapon, pAI->GetAIBlackBoard()->GetBBPrimaryWeaponProp() );
	pAnimProps->Set( kAPG_WeaponPosition, kAP_WPOS_Up );
	pAnimProps->Set( kAPG_Action, kAP_ACT_Death );
	pAnimProps->Set( kAPG_Activity, eVehicle );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionDeathOnVehicle::IsActionComplete
//
//	PURPOSE:	Determine if action has completed.
//
// ----------------------------------------------------------------------- //

bool CAIActionDeathOnVehicle::IsActionComplete( CAI* pAI )
{
	// Death is complete when the animation finishes.

	if( ( pAI->GetState() ) &&
		( pAI->GetState()->GetStateStatus() == kAIStateStatus_Complete ) )
	{
		return true;
	}

	// Death is not complete.

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionDeathOnVehicle::ApplyContextEffect
//
//	PURPOSE:	Apply affects to the real game world.
//
// ----------------------------------------------------------------------- //

void CAIActionDeathOnVehicle::ApplyContextEffect( CAI* pAI, CAIWorldState* pwsWorldStateCur, CAIWorldState* pwsWorldStateGoal )
{
	// Sanity check.

	if( !pAI )
	{
		return;
	}

	// Turn on the KeyframeToRigidBody object to separate the AI from the vehicle.

	HOBJECT hKeyframeToRigidBody = pAI->GetAIBlackBoard()->GetBBVehicleKeyframeToRigidBody();
	if( hKeyframeToRigidBody )
	{
		KeyframeToRigidBody* pKeyframeToRigidBody = (KeyframeToRigidBody*)g_pLTServer->HandleToObject( hKeyframeToRigidBody );
		g_pCmdMgr->QueueMessage( pAI, pKeyframeToRigidBody, "ON" );
	}

	// Stop updating the AI completely.

	pAI->SetUpdateAI( false );
}
