// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionDrawWeapon.cpp
//
// PURPOSE : AIActionDrawWeapon abstract class implementation
//
// CREATED : 1/29/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIActionDrawWeapon.h"
#include "AI.h"
#include "AIDB.h"
#include "AIState.h"
#include "AIStateAnimate.h"
#include "AIBlackBoard.h"
#include "AIUtils.h"
#include "AnimationContext.h"
#include "NodeTrackerContext.h"
#include "AIWeaponMgr.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionDrawWeapon, kAct_DrawWeapon );


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionDrawWeapon::Con/destructor
//
//	PURPOSE:	Con/destructor
//
// ----------------------------------------------------------------------- //

CAIActionDrawWeapon::CAIActionDrawWeapon()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionDrawWeapon::InitAction
//
//	PURPOSE:	Handle initializing the AIAction, setting the 
//				ActionRecord to use, as well as preconditions and effects.
//
// ----------------------------------------------------------------------- //

void CAIActionDrawWeapon::InitAction( AIDB_ActionRecord* pActionRecord )
{
	super::InitAction( pActionRecord );

	// Set effects.
	// Weapon is armed.

	m_wsWorldStateEffects.SetWSProp( kWSK_WeaponArmed, NULL, kWST_bool, true );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionDrawWeapon::ActivateAction
//
//	PURPOSE:	Activate action.
//
// ----------------------------------------------------------------------- //

void CAIActionDrawWeapon::ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal )
{
	super::ActivateAction( pAI, wsWorldStateGoal );

	// Set animate state.

	pAI->SetState( kState_Animate );

	// Play draw animation for the weapon type being drawn.

	EnumAnimProp eWeaponProp = g_pAIDB->GetAIConstantsRecord()->eUnarmedWeaponProp;
	const AIDB_AIWeaponRecord* pRecord = AIWeaponUtils::GetAIWeaponRecord(
		pAI->GetAIBlackBoard()->GetBBHolsterRightAIWeaponRecordID());
	
	if (pRecord)
	{
		eWeaponProp = pRecord->eAIWeaponAnimProp;
	}

	// Set draw animation.

	CAnimationProps	animProps;
	animProps.Set( kAPG_Posture, kAP_POS_Stand );
	animProps.Set( kAPG_Weapon, eWeaponProp );
	animProps.Set( kAPG_Action, kAP_ACT_Draw );

	CAIStateAnimate* pStateAnimate = (CAIStateAnimate*)( pAI->GetState() );
	pStateAnimate->SetAnimation( animProps, !LOOP );

	// Torso tracking.

	pAI->GetAIBlackBoard()->SetBBTargetTrackerFlags( kTrackerFlag_AimAt );
	pAI->GetAIBlackBoard()->SetBBFaceTarget( true );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionDrawWeapon::ValidateContextPreconditions
//
//	PURPOSE:	Determine if action has completed.
//
// ----------------------------------------------------------------------- //

bool CAIActionDrawWeapon::ValidateContextPreconditions(CAI* pAI, CAIWorldState& wzWorldStateGoal, bool bIsPlanning)
{
	// No holstered weapon to draw.

	if (kAIWeaponID_Invalid == pAI->GetAIBlackBoard()->GetBBHolsterRightAIWeaponRecordID())
	{
		return false;
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionAttack::IsActionComplete
//
//	PURPOSE:	Determine if action has completed.
//
// ----------------------------------------------------------------------- //

bool CAIActionDrawWeapon::IsActionComplete( CAI* pAI )
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

