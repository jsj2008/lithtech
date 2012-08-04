// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionIdleTurret.cpp
//
// PURPOSE : AIActionIdleTurret class implementation
//
// CREATED : 6/29/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIActionIdleTurret.h"
#include "AI.h"
#include "AIStateAnimate.h"
#include "AIBlackBoard.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionIdleTurret, kAct_IdleTurret );


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionIdleTurret::ActivateAction
//
//	PURPOSE:	Activate action.
//
// ----------------------------------------------------------------------- //

void CAIActionIdleTurret::ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal )
{
	super::ActivateAction( pAI, wsWorldStateGoal );

	// If turret was scanning, keep playing the alert 
	// scanning animation while idle.

	if( ( pAI->GetAIBlackBoard()->GetBBDestStatus() != kNav_Unset ) &&
		( pAI->GetState()->GetStateClassType() == kState_Animate ) &&
		( pAI->GetAIBlackBoard()->GetBBSensesOn() ) )
	{
		CAIStateAnimate* pStateAnimate = (CAIStateAnimate*)( pAI->GetState() );

		CAnimationProps	animProps;
		pStateAnimate->GetAnimationProps( &animProps );

		animProps.Set( kAPG_WeaponPosition, kAP_WPOS_Up );
		animProps.Set( kAPG_Action, kAP_ACT_Alert );

		pStateAnimate->SetAnimation( animProps, LOOP );
	}

	// Head tracking.

	pAI->GetAIBlackBoard()->SetBBTargetTrackerFlags( kTrackerFlag_None );
	pAI->GetAIBlackBoard()->SetBBFaceTarget( false );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionIdleTurret::ValidateAction
//
//	PURPOSE:	Returns true if AI should keep following.
//
// ----------------------------------------------------------------------- //

bool CAIActionIdleTurret::ValidateAction( CAI* pAI )
{
	if( !super::ValidateAction( pAI ) )
	{
		return false;
	}

	// Invalidate if AI just had his senses turned off.
	// This allows turret to switch from the scan animation to the limp anim.

	if( ( !pAI->GetAIBlackBoard()->GetBBSensesOn() ) &&
		( pAI->GetAnimationContext()->GetCurrentProp( kAPG_Action ) == kAP_ACT_Alert ) )
	{
		return false;
	}

	return true;
}
