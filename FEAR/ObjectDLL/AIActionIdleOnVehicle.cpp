// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionIdleOnVehicle.cpp
//
// PURPOSE : AIActionIdleOnVehicle abstract class implementation
//
// CREATED : 12/23/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIActionIdleOnVehicle.h"
#include "AI.h"
#include "AIBlackBoard.h"
#include "AIState.h"
#include "AIStateAnimate.h"
#include "AITarget.h"
#include "AIUtils.h"
#include "AnimationContext.h"
#include "NodeTrackerContext.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionIdleOnVehicle, kAct_IdleOnVehicle );


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionMountVehicle::ValidateContextPreconditions
//
//	PURPOSE:	Return true if real-time preconditions are valid.
//
// ----------------------------------------------------------------------- //

bool CAIActionIdleOnVehicle::ValidateContextPreconditions( CAI* pAI, CAIWorldState& wsWorldStateGoal, bool bIsPlanning )
{
	if( !super::ValidateContextPreconditions( pAI, wsWorldStateGoal, bIsPlanning ) )
	{
		return false;
	}

	// Action is only valid if AI is riding a Vehicle.

	SAIWORLDSTATE_PROP* pProp = pAI->GetAIWorldState()->GetWSProp( kWSK_RidingVehicle, pAI->m_hObject );
	if( ( !pProp ) ||
		( pProp->eAnimPropWSValue == kAP_None ) )
	{
		return false;
	}

	// Action is valid.

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionIdleOnVehicle::ActivateAction
//
//	PURPOSE:	Activate action.
//
// ----------------------------------------------------------------------- //

void CAIActionIdleOnVehicle::ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal )
{
	// This function did not call down to it's base class, CAIActionIdle.
	//
	// Be sure to call down to the base class however to insure that any 
	// awareness modifications are applied.
	CAIActionAbstract::ActivateAction( pAI, wsWorldStateGoal );

	// Sanity check.

	if( !pAI )
	{
		return;
	}

	// Get vehicle animProp.

	SAIWORLDSTATE_PROP* pProp = pAI->GetAIWorldState()->GetWSProp( kWSK_RidingVehicle, pAI->m_hObject );
	if( !pProp )
	{
		return;
	}
	EnumAnimProp eVehicle = pProp->eAnimPropWSValue;

	// Set animate state.

	pAI->SetState( kState_Animate );

	// Set idle animation.

	CAnimationProps	animProps;
	animProps.Set( kAPG_Posture, kAP_POS_Stand );
	animProps.Set( kAPG_Weapon, pAI->GetAIBlackBoard()->GetBBPrimaryWeaponProp() );
	animProps.Set( kAPG_WeaponPosition, kAP_WPOS_Lower );
	animProps.Set( kAPG_Action, kAP_ACT_Idle );
	animProps.Set( kAPG_Activity, eVehicle );

	CAIStateAnimate* pStateAnimate = (CAIStateAnimate*)( pAI->GetState() );
	pStateAnimate->SetAnimation( animProps, LOOP );

	// Head tracking.

	pAI->GetAIBlackBoard()->SetBBTargetTrackerFlags( kTrackerFlag_LookAt );
	pAI->GetAIBlackBoard()->SetBBFaceTarget( false );
}

