// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionDeathOnVehicleUnanimated.h
//
// PURPOSE : 
//
// CREATED : 3/23/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIActionDeathOnVehicleUnanimated.h"
#include "AI.h"
#include "AIStateAnimate.h"
#include "AIBlackBoard.h"
#include "AnimationContext.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionDeathOnVehicleUnanimated, kAct_DeathOnVehicle );

static const char* const g_szPhysicsWeightSet = "Vehicle";

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionDeathOnVehicleUnanimated::Con/destructor
//
//	PURPOSE:	Con/destructor
//
// ----------------------------------------------------------------------- //

CAIActionDeathOnVehicleUnanimated::CAIActionDeathOnVehicleUnanimated()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionDeathOnVehicleUnanimated::InitAction
//
//	PURPOSE:	Handle initializing the AIAction, setting the 
//				ActionRecord to use, as well as preconditions and effects.
//
// ----------------------------------------------------------------------- //

void CAIActionDeathOnVehicleUnanimated::InitAction( AIDB_ActionRecord* pActionRecord )
{
	super::InitAction( pActionRecord );

	// Set preconditions.

	// Set effects.
	// AI reacted to damage.

	m_wsWorldStateEffects.SetWSProp( kWSK_ReactedToWorldStateEvent, NULL, kWST_ENUM_AIWorldStateEvent, kWSE_Damage );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionDeathOnVehicleUnanimated::ValidateContextPreconditions
//
//	PURPOSE:	Return true if real-time preconditions are valid.
//
// ----------------------------------------------------------------------- //

bool CAIActionDeathOnVehicleUnanimated::ValidateContextPreconditions( CAI* pAI, CAIWorldState& wsWorldStateGoal, bool bIsPlanning )
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

	// Assert if the AI does not have the required weightset.

#if !defined(_FINAL) && !defined(_DEMO)
	HPHYSICSWEIGHTSET hSet;
	if ( g_pModelLT->FindPhysicsWeightSet( pAI->GetHOBJECT(), g_szPhysicsWeightSet, hSet) != LT_OK )
	{
		AIASSERT1( 0, pAI->GetHOBJECT(), "Failed to find the required physics weightset.  To die while on a vehicle, AIs must have a physics weightset named %s.", g_szPhysicsWeightSet );
		return false;
	}
#endif

	// Do not die.

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionDeathOnVehicleUnanimated::ActivateAction
//
//	PURPOSE:	Activate action.
//
// ----------------------------------------------------------------------- //

void CAIActionDeathOnVehicleUnanimated::ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal )
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

	// Set death animation -- continue playing the current animation.

	CAnimationProps	animProps;
	pAI->GetAnimationContext()->GetCurrentProps( &animProps );
	CAIStateAnimate* pStateAnimate = (CAIStateAnimate*)( pAI->GetState() );
	pStateAnimate->SetAnimation( animProps, !LOOP );

	// Circumvent the normal CCharacter code path for handling 
	// deaths, and creating Body objects.

	pAI->GetAIBlackBoard()->SetBBAIHandlingDeath( true );

	// Enable physics simulation

	PhysicsUtilities::SetPhysicsWeightSet( pAI->GetHOBJECT(), g_szPhysicsWeightSet, false );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionDeathOnVehicleUnanimated::IsActionComplete
//
//	PURPOSE:	Determine if action has completed.
//
// ----------------------------------------------------------------------- //

bool CAIActionDeathOnVehicleUnanimated::IsActionComplete( CAI* pAI )
{
	// Action is complete if the state didn't just change (at least one frame 
	// has passed in this action)

	return ( g_pLTServer->GetTime() != pAI->GetAIBlackBoard()->GetBBStateChangeTime() );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionDeathOnVehicleUnanimated::ApplyContextEffect
//
//	PURPOSE:	Apply affects to the real game world.
//
// ----------------------------------------------------------------------- //

void CAIActionDeathOnVehicleUnanimated::ApplyContextEffect( CAI* pAI, CAIWorldState* pwsWorldStateCur, CAIWorldState* pwsWorldStateGoal )
{
	// Sanity check.

	if( !pAI )
	{
		return;
	}

	// Revert to normal death handling.

	pAI->GetAIBlackBoard()->SetBBAIHandlingDeath( false );

	// Put the AI into full ragdoll/perform standard death handling.
	// Don't clamp the death velocities, as they are mostly external (from 
	// movement of the AI, not from AI animation).

	pAI->SetDeathAnimation( false );
}
