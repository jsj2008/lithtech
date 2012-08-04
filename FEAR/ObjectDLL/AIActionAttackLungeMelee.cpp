// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionAttackLungeMelee.cpp
//
// PURPOSE : AIActionAttackLungeMelee class implementation
//
// CREATED : 9/28/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIActionAttackLungeMelee.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionAttackLungeMelee, kAct_AttackLungeMelee );


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionAttackLungeMelee::ActivateAction
//
//	PURPOSE:	Activate action.
//
// ----------------------------------------------------------------------- //

void CAIActionAttackLungeMelee::ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal )
{
	super::ActivateAction( pAI, wsWorldStateGoal );

	// Face the target.

	pAI->GetAIBlackBoard()->SetBBFaceTarget( true );

	// Do not use touch handling

	pAI->GetAIBlackBoard()->SetBBHandleTouch( kTouch_None );

	// Do not update target aim tests.

	pAI->GetAIBlackBoard()->SetBBUpdateTargetAim( false );

	// Do not move thru the target.

	pAI->GetAIMovement()->AllowTargetPenetration( false );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionAttackLungeMelee::DeactivateAction
//
//	PURPOSE:	Deactivate action.
//
// ----------------------------------------------------------------------- //

void CAIActionAttackLungeMelee::DeactivateAction( CAI* pAI )
{
	super::DeactivateAction( pAI );

	// Sanity check.

	if( !pAI )
	{
		return;
	}

	// Default behavior.

	pAI->GetAIBlackBoard()->SetBBUpdateTargetAim( true );
	pAI->GetAIMovement()->AllowTargetPenetration( true );
}

