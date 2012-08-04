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
#include "AIActionAttackMelee.h"
#include "AI.h"
#include "AIDB.h"
#include "AITarget.h"
#include "AIBrain.h"
#include "AIWeaponMgr.h"
#include "AIBlackBoard.h"
#include "AnimationContext.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionAttackMelee, kAct_AttackMelee );

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionAttackMelee::CAIActionAttackMelee
//
//	PURPOSE:	Handle constructing the Melee action
//
// ----------------------------------------------------------------------- //

CAIActionAttackMelee::CAIActionAttackMelee()
{
	SetValidateVisibility(false);
	m_bInterruptActionIfEnemyIsOutOfRange = false;
	m_bDistanceClosingAttack = false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionAttackMelee::ValidateContextPreconditions
//
//	PURPOSE:	Return true if real-time preconditions are valid.
//
// ----------------------------------------------------------------------- //

bool CAIActionAttackMelee::ValidateContextPreconditions( CAI* pAI, CAIWorldState& wsWorldStateGoal, bool bIsPlanning )
{
	// AI doesn't have a target.

	if (!pAI->HasTarget( kTarget_Character ))
	{
		return false;
	}

	// Target is not visible.

	if (!pAI->GetAIBlackBoard()->GetBBTargetVisibleFromWeapon())
	{
		return false;
	}

	// AI does not have a weapon of the correct type

	if (!AIWeaponUtils::HasWeaponType(pAI, GetWeaponType(), bIsPlanning))
	{
		return false;
	}

	// AI does not have any ammo for this weapon.

	if ( !AIWeaponUtils::HasAmmo( pAI, GetWeaponType(), bIsPlanning ) )
	{
		return false;
	}

	// Target is not in range

	if ( !m_bDistanceClosingAttack 
		&& !AIWeaponUtils::IsInRange(pAI, GetWeaponType(), bIsPlanning) )
	{
		return false;
	}

	// Target is too close, and this is a distance closing attack.  
	// TODO: This currently assumes the weapon is already drawn.  If it is not,
	// this will fail.  This is tolerable for now as other behaviors will insure 
	// the weapon is drawn, but it is inconsistent with typical AI behaviors.

	if ( m_bDistanceClosingAttack )
	{
		ENUM_RangeStatus eStatus = pAI->GetAIBlackBoard()->GetBBWeaponStatus( GetWeaponType() );
		if( ( eStatus != kRangeStatus_Ok && eStatus != kRangeStatus_TooFar ) )
		{
			return false;
		}
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionAttackMelee::IsActionInterruptible
//
//	PURPOSE:	Return true if the action is interruptible.  If an AI is 
//				aiming, waiting to be able to attack again for whatever 
//				reason, this plan is interruptible.
//
// ----------------------------------------------------------------------- //

bool CAIActionAttackMelee::IsActionInterruptible(CAI* pAI)
{
	if (pAI->GetAnimationContext()->IsPropSet(kAPG_Action, kAP_ACT_Aim))
	{
		return true;
	}
	else
	{
		return m_pActionRecord->bActionIsInterruptible;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionAttackMelee::ActivateAction
//
//	PURPOSE:	Activate action.
//
// ----------------------------------------------------------------------- //

void CAIActionAttackMelee::ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal )
{
	super::ActivateAction( pAI, wsWorldStateGoal );

	// Sanity check.

	if( !pAI )
	{
		return;
	}

	// Do not move thru the target.

	pAI->GetAIMovement()->AllowTargetPenetration( false );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionAttackMelee::DeactivateAction
//
//	PURPOSE:	Deactivate action.
//
// ----------------------------------------------------------------------- //

void CAIActionAttackMelee::DeactivateAction( CAI* pAI )
{
	super::DeactivateAction( pAI );

	// Sanity check.

	if( !pAI )
	{
		return;
	}

	// Default behavior.

	pAI->GetAIMovement()->AllowTargetPenetration( true );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionAttackMelee::SetAttackAnimProps
//
//	PURPOSE:	Set animation props.
//
// ----------------------------------------------------------------------- //

void CAIActionAttackMelee::SetAttackAnimProps( CAI* pAI, CAnimationProps* pProps )
{
	super::SetAttackAnimProps( pAI, pProps );

	// Sanity check.

	if( !pProps )
	{
		return;
	}

	pProps->Set( kAPG_Action, kAP_ACT_AttackMelee );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionAttackMelee::GetLoopAttackAnimation
//
//	PURPOSE:	For now, melee attacks never loop.  This resolves the use 
//				of a single weapon type (ie submachinegun) for either 
//				firing/looping or melee/non-looping
//
// ----------------------------------------------------------------------- //

bool CAIActionAttackMelee::GetLoopAttackAnimation( CAI* pAI ) const
{
	return false;
}
