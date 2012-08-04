// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionAttackMeleeFromRun.cpp
//
// PURPOSE : 
//
// CREATED : 11/03/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIActionAttackMeleeFromRun.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionAttackMeleeFromRun, kAct_AttackMeleeFromRun );

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionAttackMeleeFromRun::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAIActionAttackMeleeFromRun::CAIActionAttackMeleeFromRun()
{
}

CAIActionAttackMeleeFromRun::~CAIActionAttackMeleeFromRun()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionAttackMeleeFromRun::ValidateContextPreconditions
//
//	PURPOSE:	Return true if real-time preconditions are valid.
//
// ----------------------------------------------------------------------- //

bool CAIActionAttackMeleeFromRun::ValidateContextPreconditions( CAI* pAI, CAIWorldState& wsWorldStateGoal, bool bIsPlanning )
{
	if ( !super::ValidateContextPreconditions( pAI, wsWorldStateGoal, bIsPlanning ) )
	{
		return false;
	}

	// Fail if the AI is not moving.

	if ( kAP_None == pAI->GetAnimationContext()->GetCurrentProp( kAPG_Movement ) )
	{
		return false;
	}

	// Fail if there are no matching animations.

	CAnimationProps	animProps;
	SetAttackAnimProps( pAI, &animProps );
	uint32 nAnimations = pAI->GetAnimationContext()->CountAnimations( animProps );
	if ( 0 == nAnimations )
	{
		return false;
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionAttackMeleeFromRun::SetAttackAnimProps
//
//	PURPOSE:	Set animation props.
//
// ----------------------------------------------------------------------- //

void CAIActionAttackMeleeFromRun::SetAttackAnimProps( CAI* pAI, CAnimationProps* pProps )
{
	super::SetAttackAnimProps( pAI, pProps );

	// Sanity check.

	if( !pProps )
	{
		return;
	}

	pProps->Set( kAPG_Action, kAP_ACT_AttackMeleeFromRun );
	pProps->Set( kAPG_Activity, pAI->GetAnimationContext()->GetCurrentProp( kAPG_Activity ) ); 
	pProps->Set( kAPG_Weapon,  pAI->GetAIBlackBoard()->GetBBPrimaryWeaponProp() );
}
