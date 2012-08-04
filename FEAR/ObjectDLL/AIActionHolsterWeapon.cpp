// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionHolsterWeapon.cpp
//
// PURPOSE : AIActionHolsterWeapon abstract class implementation
//
// CREATED : 1/29/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

// Includes required for AIActionHolsterWeapon.h

#include "Stdafx.h"
#include "AIActionAbstract.h"
#include "AIActionHolsterWeapon.h"

// Includes required for AIActionHolsterWeapon.cpp

#include "AI.h"
#include "AIState.h"
#include "AIStateAnimate.h"
#include "AIBlackBoard.h"
#include "AIUtils.h"
#include "NodeTrackerContext.h"
#include "AnimationContext.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionHolsterWeapon, kAct_HolsterWeapon );


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionHolsterWeapon::Con/destructor
//
//	PURPOSE:	Con/destructor
//
// ----------------------------------------------------------------------- //

CAIActionHolsterWeapon::CAIActionHolsterWeapon()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionHolsterWeapon::InitAction
//
//	PURPOSE:	Handle initializing the AIAction, setting the 
//				ActionRecord to use, as well as preconditions and effects.
//
// ----------------------------------------------------------------------- //

void CAIActionHolsterWeapon::InitAction( AIDB_ActionRecord* pActionRecord )
{
	super::InitAction( pActionRecord );

	// Set effects.
	// Weapon is not armed.

	m_wsWorldStateEffects.SetWSProp( kWSK_WeaponArmed, NULL, kWST_bool, false );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionHolsterWeapon::ActivateAction
//
//	PURPOSE:	Activate action.
//
// ----------------------------------------------------------------------- //

void CAIActionHolsterWeapon::ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal )
{
	super::ActivateAction( pAI, wsWorldStateGoal );

	// Set animate state.

	pAI->SetState( kState_Animate );

	// Set holster animation.

	CAnimationProps	animProps;
	animProps.Set( kAPG_Posture, kAP_POS_Stand );
	animProps.Set( kAPG_Weapon, pAI->GetAIBlackBoard()->GetBBPrimaryWeaponProp() );
	animProps.Set( kAPG_Action, kAP_ACT_Holster );

	CAIStateAnimate* pStateAnimate = (CAIStateAnimate*)( pAI->GetState() );
	pStateAnimate->SetAnimation( animProps, !LOOP );

	// Torso tracking.

	pAI->GetAIBlackBoard()->SetBBTargetTrackerFlags( kTrackerFlag_None );
	pAI->GetAIBlackBoard()->SetBBFaceTarget( false );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionHolsterWeapon::IsActionComplete
//
//	PURPOSE:	Determine if action has completed.
//
// ----------------------------------------------------------------------- //

bool CAIActionHolsterWeapon::IsActionComplete( CAI* pAI )
{
	// Holstering is complete when state has completed.

	if( ( pAI->GetState() ) &&
		( pAI->GetState()->GetStateStatus() == kAIStateStatus_Complete ) )
	{
		return true;
	}

	// Did not need to holster.

	if( !pAI->GetAnimationContext()->IsLocked() )
	{
		SAIWORLDSTATE_PROP* pProp = pAI->GetAIWorldState()->GetWSProp( kWSK_WeaponArmed, pAI->m_hObject );
		if( !pProp->bWSValue )
		{
			return true;
		}
	}

	// Holstering is not complete.

	return false;
}



