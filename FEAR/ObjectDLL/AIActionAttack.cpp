// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionAttack.cpp
//
// PURPOSE : AIActionAttack class implementation
//
// CREATED : 1/29/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIActionAttack.h"
#include "AI.h"
#include "AIState.h"
#include "AIStateAnimate.h"
#include "AITarget.h"
#include "AIBrain.h"
#include "AIWeaponMgr.h"
#include "AIBlackBoard.h"
#include "AISoundMgr.h"
#include "AICoordinator.h"
#include "AnimationContext.h"
#include "NodeTrackerContext.h"
#include "PlayerObj.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionAttack, kAct_Attack );


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionAttack::Con/destructor
//
//	PURPOSE:	Con/destructor
//
// ----------------------------------------------------------------------- //

CAIActionAttack::CAIActionAttack()
{
	m_bPlayAISound = true;
	m_bValidateVisibility = true;
	m_bInterruptActionIfEnemyIsOutOfRange = false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionAttack::InitAction
//
//	PURPOSE:	Handle initializing the AIAction, setting the 
//				ActionRecord to use, as well as preconditions and effects.
//
// ----------------------------------------------------------------------- //

void CAIActionAttack::InitAction( AIDB_ActionRecord* pActionRecord )
{
	super::InitAction( pActionRecord );

	// Set preconditions.
	// Weapon must be armed.
	// Weapon must be loaded.
	// Must not be mounting a node.

	m_wsWorldStatePreconditions.SetWSProp( kWSK_WeaponArmed, NULL, kWST_bool, true );
	m_wsWorldStatePreconditions.SetWSProp( kWSK_WeaponLoaded, NULL, kWST_bool, true );
	m_wsWorldStatePreconditions.SetWSProp( kWSK_MountedObject, NULL, kWST_HOBJECT, 0 );

	// Set effects.
	// Target is dead.

	m_wsWorldStateEffects.SetWSProp( kWSK_TargetIsDead, NULL, kWST_bool, true );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionAttack::ValidateContextPreconditions
//
//	PURPOSE:	Return true if real-time preconditions are valid.
//
// ----------------------------------------------------------------------- //

bool CAIActionAttack::ValidateContextPreconditions( CAI* pAI, CAIWorldState& wsWorldStateGoal, bool bIsPlanning )
{
	// No target.

	if( !pAI->HasTarget( kTarget_Character | kTarget_CombatOpportunity | kTarget_Object ) )
	{
		return false;
	}

	// Target is not visible.

	if( m_bValidateVisibility )
	{
		if( ( !pAI->GetAIBlackBoard()->GetBBTargetVisibleFromWeapon() ) &&
			( !IsAI( pAI->GetTarget()->GetVisionBlocker() ) ) )
		{
			return false;
		}
	}

	// AI does not have a weapon of the correct type

	if (!AIWeaponUtils::HasWeaponType(pAI, GetWeaponType(), bIsPlanning))
	{
		return false;
	}

	// Target is not in range.

	if (!AIWeaponUtils::IsInRange(pAI, GetWeaponType(), bIsPlanning))
	{
		return false;
	}

	// AI does not have any ammo required by this weapon type.

	if ( !AIWeaponUtils::HasAmmo(pAI, GetWeaponType(), bIsPlanning ) )
	{
		return false;
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionAttack::ActivateAction
//
//	PURPOSE:	Activate action.
//
// ----------------------------------------------------------------------- //

void CAIActionAttack::ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal )
{
	super::ActivateAction( pAI, wsWorldStateGoal );

	// Set our weapon of the desired type as current weapon.

	pAI->SetCurrentWeapon( GetWeaponType() );

	// Set animate state.

	pAI->SetState( kState_Animate );

	// Set attack animation.

	CAnimationProps	animProps;
	SetAttackAnimProps( pAI, &animProps );

	CAIStateAnimate* pStateAnimate = (CAIStateAnimate*)( pAI->GetState() );
	pStateAnimate->SetAnimation( animProps, GetLoopAttackAnimation( pAI ) );

	// Turn off any automatic reloading that may have been previously set.

	pAI->GetAIBlackBoard()->SetBBAutoReload( false );

	// Get the target.

	HOBJECT hTarget = NULL;
	if( pAI->HasTarget( kTarget_Character | kTarget_Object | kTarget_CombatOpportunity) )
	{
		hTarget = pAI->GetAIBlackBoard()->GetBBTargetObject();
	}

	// Play attack sound.
	// Only play if we've been targeting someone for at least 5 seconds.
	// This ensures we first play higher priority reaction sounds.

	if( ( m_bPlayAISound && pAI->GetAIBlackBoard()->GetBBTargetVisibleFromWeapon() ) &&
		( pAI->GetAIBlackBoard()->GetBBTargetChangeTime() < g_pLTServer->GetTime() - 10.f ) )
	{
		if( IsTurret( hTarget ) )
		{
			g_pAISoundMgr->RequestAISound( pAI->m_hObject, kAIS_TurretStress, kAISndCat_Event, hTarget, 0.f );
		}
		else if( g_pAICoordinator->FindAlly( pAI->m_hObject, NULL ) )
		{
			g_pAISoundMgr->RequestAISound( pAI->m_hObject, kAIS_Attack, kAISndCat_Combat, hTarget, 0.f );
		}
	}

	// Torso tracking.

	pAI->GetAIBlackBoard()->SetBBTargetTrackerFlags( kTrackerFlag_AimAt );
	pAI->GetAIBlackBoard()->SetBBFaceTarget( true );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionAttackLungeMelee::DeactivateAction
//
//	PURPOSE:	Deactivate action.
//
// ----------------------------------------------------------------------- //

void CAIActionAttack::DeactivateAction( CAI* pAI )
{
	super::DeactivateAction( pAI );

	// Sanity check.

	if( !pAI )
	{
		return;
	}

	// Return to the primary weapon.

	pAI->SetCurrentWeapon( pAI->GetAIBlackBoard()->GetBBPrimaryWeaponType() );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionAttack::SetAttackAnimProps
//
//	PURPOSE:	Set animation props.
//
// ----------------------------------------------------------------------- //

void CAIActionAttack::SetAttackAnimProps( CAI* pAI, CAnimationProps* pProps )
{
	// Sanity check.

	if( !( pAI && pProps ) )
	{
		return;
	}

	pProps->Set( kAPG_Posture, kAP_POS_Stand );
	pProps->Set( kAPG_WeaponPosition, kAP_WPOS_Up );
	pProps->Set( kAPG_Action, kAP_ACT_Fire );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionAttack::ValidateAction
//
//	PURPOSE:	Return true if action is still valid.
//
// ----------------------------------------------------------------------- //

bool CAIActionAttack::ValidateAction( CAI* pAI )
{
	if( !super::ValidateAction( pAI ) )
	{
		return false;
	}

	// Target is not visible.
	// And target is not obscured by an AI.

	if( m_bValidateVisibility)
	{
		if ( !pAI->GetAIBlackBoard()->GetBBTargetVisibleFromWeapon() 
			&& ( !IsAI( pAI->GetTarget()->GetVisionBlocker() ) ) )
		{
			return false;
		}
	}
 
	// Target is not in range.

	if ( m_bInterruptActionIfEnemyIsOutOfRange )
	{
		if( pAI->GetAIBlackBoard()->GetBBWeaponStatus(GetWeaponType()) != kRangeStatus_Ok )
		{
			return false;
		}
	}

	// Weapon is unloaded.

	SAIWORLDSTATE_PROP* pProp = pAI->GetAIWorldState()->GetWSProp( kWSK_WeaponLoaded, pAI->m_hObject );
	if( pProp && !pProp->bWSValue )
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

bool CAIActionAttack::IsActionComplete( CAI* pAI )
{
	// Attack is complete if state is complete.

	if( ( pAI->GetState() ) &&
		( pAI->GetState()->GetStateStatus() == kAIStateStatus_Complete ) )
	{
		return true;
	}

	// Attack is not complete.

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionAttack::GetLoopAttackAnimation
//
//	PURPOSE:	Returns true if the attack animation should loop, false 
//				if it should not.
//
// ----------------------------------------------------------------------- //

bool CAIActionAttack::GetLoopAttackAnimation( CAI* pAI ) const
{
	// Determine if the attack should be played looping or not.  This is safe
	// as we set the CurrentWeapon to the actions WeaponType earlier in this 
	// function

	const AIDB_AIWeaponRecord* pRecord = g_pAIDB->GetAIWeaponRecord(
		pAI->GetAIBlackBoard()->GetBBCurrentAIWeaponRecordID());

	return ( pRecord ? pRecord->bLoopFireAnimation : true );
}
