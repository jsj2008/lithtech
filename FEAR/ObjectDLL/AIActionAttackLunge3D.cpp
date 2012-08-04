// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionAttackLunge3D.cpp
//
// PURPOSE : AIActionAttackLunge3D class implementation
//
// CREATED : 9/01/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIActionAttackLunge3D.h"
#include "AI.h"
#include "AIStateGoto.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionAttackLunge3D, kAct_AttackLunge3D );


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionAttackLunge3D::GetLungeMaxDist
//
//	PURPOSE:	Return the max distance AI can lunge.
//
// ----------------------------------------------------------------------- //

float CAIActionAttackLunge3D::GetLungeMaxDist( CAI* pAI, CAIWMFact* pDesireFact )
{
	if( !( pAI && pDesireFact ) )
	{
		return 0.f;
	}

	LTVector vTarget = pAI->GetAIBlackBoard()->GetBBTargetPosition();
	return vTarget.Dist( pAI->GetPosition() ) + 100.f;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionAttackLunge3D::ActivateAction
//
//	PURPOSE:	Activate action.
//
// ----------------------------------------------------------------------- //

void CAIActionAttackLunge3D::ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal )
{
	super::ActivateAction( pAI, wsWorldStateGoal );

	if( !pAI->HasTarget( kTarget_All ) )
	{
		return;
	}
	
	// Set the destination.

	LTVector vTarget = pAI->GetAIBlackBoard()->GetBBTargetPosition();
	LTVector vDir = vTarget - pAI->GetPosition();
	vDir.y = 0.f;
	vDir.Normalize();
	LTVector vDest = vTarget + ( vDir * 100.f );
	vDest.y += 50.f;

	// Turn on movement scaling.

	pAI->GetAIBlackBoard()->SetBBInterpolateMovementHeight( false );
	pAI->GetAIBlackBoard()->SetBBScaleMovement( true );

	// Set a destination, to force the animation to scale.

	pAI->GetAIMovement()->SetMovementDest( vDest );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionAttackLungeUncloaked::SetAttackAnimProps
//
//	PURPOSE:	Set animation props.
//
// ----------------------------------------------------------------------- //

void CAIActionAttackLunge3D::SetAttackAnimProps( CAI* pAI, CAnimationProps* pProps )
{
	super::SetAttackAnimProps( pAI, pProps );

	// Sanity check.

	if( !( pAI && pProps ) )
	{
		return;
	}

	// Select and up or down animation.

	LTVector vTarget = pAI->GetAIBlackBoard()->GetBBTargetPosition();
	if( pAI->GetPosition().y > vTarget.y )
	{
		pProps->Set( kAPG_MovementDir, kAP_MDIR_Down );
	}
	else {
		pProps->Set( kAPG_MovementDir, kAP_MDIR_Up );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionAttackLunge3D::DeactivateAction
//
//	PURPOSE:	Deactivate action.
//
// ----------------------------------------------------------------------- //

void CAIActionAttackLunge3D::DeactivateAction( CAI* pAI )
{
	super::DeactivateAction( pAI );

	// Sanity check.

	if( !pAI )
	{
		return;
	}

	// Turn off movement interpolation and scaling.

	pAI->GetAIBlackBoard()->SetBBInterpolateMovementHeight( false );
	pAI->GetAIBlackBoard()->SetBBScaleMovement( false );
}

