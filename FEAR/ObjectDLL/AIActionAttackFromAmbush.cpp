// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionAttackFromAmbush.cpp
//
// PURPOSE : AIActionAttackFromAmbush abstract class implementation
//
// CREATED : 6/21/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIActionAttackFromAmbush.h"
#include "AI.h"
#include "AIBlackBoard.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionAttackFromAmbush, kAct_AttackFromAmbush );


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionAttackFromAmbush::Con/destructor
//
//	PURPOSE:	Con/destructor
//
// ----------------------------------------------------------------------- //

CAIActionAttackFromAmbush::CAIActionAttackFromAmbush()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionAttackFromAmbush::ValidateContextPreconditions
//
//	PURPOSE:	Return true if real-time preconditions are valid.
//
// ----------------------------------------------------------------------- //

bool CAIActionAttackFromAmbush::ValidateContextPreconditions( CAI* pAI, CAIWorldState& wsWorldStateGoal, bool bIsPlanning )
{
	if( !super::ValidateContextPreconditions( pAI, wsWorldStateGoal, bIsPlanning ) )
	{
		return false;
	}

	// Only ambush when the target is not fully visible.
	// The only purpose of AttackFromAmbush is to have the AI waiting in a ready position.

	if( pAI->GetAIBlackBoard()->GetBBTargetVisibleFromWeapon() )
	{
		return false;
	}

	// Preconditions are valid.

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionAttackFromAmbush::SetAttackAnimProps
//
//	PURPOSE:	Set animation props.
//
// ----------------------------------------------------------------------- //

void CAIActionAttackFromAmbush::SetAttackAnimProps( CAI* pAI, CAnimationProps* pProps )
{
	// Sanity check.

	if( !( pAI && pProps ) )
	{
		return;
	}

	super::SetAttackAnimProps( pAI, pProps );

	// Do not actually fire from ambush.
	// Just wait in a ready position.

	pProps->Set( kAPG_Action, kAP_ACT_Idle );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionAttackFromAmbush::ValidateAction
//
//	PURPOSE:	Return true if action is still valid.
//
// ----------------------------------------------------------------------- //

bool CAIActionAttackFromAmbush::ValidateAction( CAI* pAI )
{
	if( !super::ValidateAction( pAI ) )
	{
		return false;
	}

	// Ambushing bails when the target is visible.
	// The only purpose of AttackFromAmbush is to have the AI waiting in a ready position.

	if( pAI->GetAIBlackBoard()->GetBBTargetVisibleFromWeapon() )
	{
		return false;
	}

	// Action is still valid.

	return true;
}

