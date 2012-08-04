// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionInstantDeath.cpp
//
// PURPOSE : AIActionInstantDeath class implementation
//
// CREATED : 03/29/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIActionInstantDeathKnockedDown.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionInstantDeathKnockedDown, kAct_InstantDeathKnockedDown );


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionInstantDeathKnockedDown::ValidateContextPreconditions
//
//	PURPOSE:	Return true if real-time preconditions are valid.
//
// ----------------------------------------------------------------------- //

bool CAIActionInstantDeathKnockedDown::ValidateContextPreconditions( CAI* pAI, CAIWorldState& wsWorldStateGoal, bool bIsPlanning )
{
	// Intentionally do NOT call super::ValidateContextPreconditions.
	// This action only handles death while knocked down.

	// No destructible.

	CDestructible* pDestructible = pAI->GetDestructible();
	if( !pDestructible )
	{
		return false;
	}

	// Don't die instantly if NeverDestroy is set.

	if( pDestructible->GetNeverDestroy() )
	{
		return false;
	}

	// Don't die if damage was not recent.

	if( pDestructible->GetLastDamageTime() + 0.3f < g_pLTServer->GetTime() )
	{
		return false;
	}

	// Die instantly if knocked down.

	EnumAnimProp eActivity = pAI->GetAnimationContext()->GetCurrentProp( kAPG_Activity );
	if( ( eActivity == kAP_ATVT_KnockDown ) &&
		( !pAI->GetAnimationContext()->IsTransitioning() ) )
	{
		return true;
	}

	// Do not die instantly.

	return false;
}
