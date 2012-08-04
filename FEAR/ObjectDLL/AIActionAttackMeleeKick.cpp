// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionAttackMeleeKick.cpp
//
// PURPOSE : 
//
// CREATED : 2/17/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIActionAttackMeleeKick.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionAttackMeleeKick, kAct_AttackMeleeKick );

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionAttackMeleeKick::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAIActionAttackMeleeKick::CAIActionAttackMeleeKick()
{
}

CAIActionAttackMeleeKick::~CAIActionAttackMeleeKick()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionAttackMeleeKick::ValidateContextPreconditions
//
//	PURPOSE:	Return true if real-time preconditions are valid.
//
// ----------------------------------------------------------------------- //

bool CAIActionAttackMeleeKick::ValidateContextPreconditions( CAI* pAI, CAIWorldState& wsWorldStateGoal, bool bIsPlanning )
{
	if ( !super::ValidateContextPreconditions( pAI, wsWorldStateGoal, bIsPlanning ) )
	{
		return false;
	}

	// Fail if the AI does not have a ranged weapon or if the AI has a melee 
	// weapon.

	if ( !AIWeaponUtils::HasWeaponType( pAI, kAIWeaponType_Ranged, AIWEAP_CHECK_HOLSTER ) )
	{
		return false;
	}

	return true;
}
