// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionAttackMeleeLong.cpp
//
// PURPOSE : AIActionAttackMeleeLong class implementation
//
// CREATED : 10/04/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIActionAttackMeleeLong.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionAttackMeleeLong, kAct_AttackMeleeLong );


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionAttackMeleeLong::ActivateAction
//
//	PURPOSE:	Activate action.
//
// ----------------------------------------------------------------------- //

void CAIActionAttackMeleeLong::ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal )
{
	super::ActivateAction( pAI, wsWorldStateGoal );

	// Face the target.

	pAI->GetAIBlackBoard()->SetBBFaceTarget( true );

	// Do not use touch handling

	pAI->GetAIBlackBoard()->SetBBHandleTouch( kTouch_None );

	// Do not move thru the target.

	pAI->GetAIMovement()->AllowTargetPenetration( false );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionAttackMeleeLong::DeactivateAction
//
//	PURPOSE:	Deactivate action.
//
// ----------------------------------------------------------------------- //

void CAIActionAttackMeleeLong::DeactivateAction( CAI* pAI )
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

