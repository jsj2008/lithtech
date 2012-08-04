// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionAttackBurstLimited.cpp
//
// PURPOSE : AIActionAttackBurstLimited class implementation
//
// CREATED : 4/06/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIActionAttackBurstLimited.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionAttackBurstLimited, kAct_AttackBurstLimited );


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionAttackBurstLimited::ValidateContextPreconditions
//
//	PURPOSE:	Return true if real-time preconditions are valid.
//
// ----------------------------------------------------------------------- //

bool CAIActionAttackBurstLimited::ValidateContextPreconditions( CAI* pAI, CAIWorldState& wsWorldStateGoal, bool bIsPlanning )
{
	if( !super::ValidateContextPreconditions( pAI, wsWorldStateGoal, bIsPlanning ) )
	{
		return false;
	}

	// Action is invalid if weapon is not ready to fire (due to burst rate).

	return pAI->GetAIWeaponMgr()->IsAIWeaponReadyToFire( kAIWeaponType_Ranged );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionAttackBurstLimited::ValidateAction
//
//	PURPOSE:	Return true if action is still valid.
//
// ----------------------------------------------------------------------- //

bool CAIActionAttackBurstLimited::ValidateAction( CAI* pAI )
{
	if( !super::ValidateAction( pAI ) )
	{
		return false;
	}

	// Action is invalid if weapon is not ready to fire (due to burst rate).

	return pAI->GetAIWeaponMgr()->IsAIWeaponReadyToFire( kAIWeaponType_Ranged );
}

