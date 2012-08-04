// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionReload.cpp
//
// PURPOSE : AIActionReload class implementation
//
// CREATED : 1/29/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIActionReload.h"
#include "AI.h"
#include "AIState.h"
#include "AIStateAnimate.h"
#include "AIBlackBoard.h"
#include "AIUtils.h"
#include "NodeTrackerContext.h"
#include "AnimationContext.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionReload, kAct_Reload );


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionReload::Con/destructor
//
//	PURPOSE:	Con/destructor
//
// ----------------------------------------------------------------------- //

CAIActionReload::CAIActionReload()
{
	m_ePosture = kAP_POS_Stand;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionReload::InitAction
//
//	PURPOSE:	Handle initializing the AIAction, setting the 
//				ActionRecord to use, as well as preconditions and effects.
//
// ----------------------------------------------------------------------- //

void CAIActionReload::InitAction( AIDB_ActionRecord* pActionRecord )
{
	super::InitAction( pActionRecord );

	// Set preconditions.
	// Weapon must be armed.
	// Weapon must be unloaded.

	m_wsWorldStatePreconditions.SetWSProp( kWSK_WeaponArmed, NULL, kWST_bool, true );

	// Set effects.
	// Weapon is loaded.

	m_wsWorldStateEffects.SetWSProp( kWSK_WeaponLoaded, NULL, kWST_bool, true );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionReload::ValidateContextPreconditions
//
//	PURPOSE:	Return true if real-time preconditions are valid.
//
// ----------------------------------------------------------------------- //

bool CAIActionReload::ValidateContextPreconditions( CAI* pAI, CAIWorldState& wsWorldStateGoal, bool bIsPlanning )
{
	if (!super::ValidateContextPreconditions(pAI, wsWorldStateGoal, bIsPlanning))
	{
		return false;
	}

	// No such weapon type.

	AIDB_AIWeaponRecord* pWeaponRecord = g_pAIDB->GetAIWeaponRecord(pAI->GetAIBlackBoard()->GetBBCurrentAIWeaponRecordID());
	if (!pWeaponRecord)
	{
		return false;
	}

	// No ammo of this type.
        
	if ( !AIWeaponUtils::HasAmmo(pAI, pWeaponRecord->eAIWeaponType, bIsPlanning) )
	{
		return false;
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionReload::ActivateAction
//
//	PURPOSE:	Activate action.
//
// ----------------------------------------------------------------------- //

void CAIActionReload::ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal )
{
	super::ActivateAction( pAI, wsWorldStateGoal );

	// Reload our randed weapon.

	pAI->SetCurrentWeapon( kAIWeaponType_Ranged );

	// Set animate state.
	
	pAI->SetState( kState_Animate );

	// Set reload animation.

	CAnimationProps	animProps;
	animProps.Set( kAPG_Posture, m_ePosture );
	animProps.Set( kAPG_Weapon, pAI->GetAIBlackBoard()->GetBBPrimaryWeaponProp() );
	animProps.Set( kAPG_WeaponPosition, kAP_WPOS_Up );
	animProps.Set( kAPG_Action, kAP_ACT_Reload );

	CAIStateAnimate* pStateAnimate = (CAIStateAnimate*)( pAI->GetState() );
	pStateAnimate->SetAnimation( animProps, !LOOP );

	// Torso tracking.

	pAI->GetAIBlackBoard()->SetBBTargetTrackerFlags( kTrackerFlag_AimAt );
	pAI->GetAIBlackBoard()->SetBBFaceTarget( true );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionReload::IsActionComplete
//
//	PURPOSE:	Determine if action has completed.
//
// ----------------------------------------------------------------------- //

bool CAIActionReload::IsActionComplete( CAI* pAI )
{
	// Reloading is complete when the animation finishes.

	if( ( pAI->GetState() ) &&
		( pAI->GetState()->GetStateStatus() == kAIStateStatus_Complete ) )
	{
		return true;
	}

	// Reloading is not complete.

	return false;
}
