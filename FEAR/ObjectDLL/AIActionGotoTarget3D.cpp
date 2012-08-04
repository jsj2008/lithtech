// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionGotoTarget3D.cpp
//
// PURPOSE : AIActionGotoTarget3D class implementation
//
// CREATED : 9/01/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIActionGotoTarget3D.h"
#include "AI.h"
#include "AIStateGoto.h"
#include "AIPathMgrNavMesh.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionGotoTarget3D, kAct_GotoTarget3D );


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionGotoTarget3D::ActivateAction
//
//	PURPOSE:	Activate action.
//
// ----------------------------------------------------------------------- //

void CAIActionGotoTarget3D::ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal )
{
	super::ActivateAction( pAI, wsWorldStateGoal );

	// Sanity check.

	if( !( pAI && pAI->GetState() && pAI->GetState()->GetStateClassType() == kState_Goto ) )
	{
		return;
	}

	// Aim 3/4 of the way up the target object.

	HOBJECT hTarget = pAI->GetAIBlackBoard()->GetBBTargetObject();
	if( hTarget )
	{
		LTVector vOffset;
		g_pPhysicsLT->GetObjectDims( hTarget, &vOffset );
		vOffset.x = 0.f;
		vOffset.z = 0.f;
		vOffset.y *= 0.5f;

		CAIStateGoto* pGoto = (CAIStateGoto*)pAI->GetState();
		pGoto->SetDestObject( hTarget, vOffset );
	}

	// If there is not a straight path to the destination,
	// just linearly interpolate the height.

	LTVector vDest = pAI->GetAIBlackBoard()->GetBBDest();
	if( !g_pAIPathMgrNavMesh->StraightPathExists( pAI, pAI->GetCharTypeMask(), pAI->GetPosition(), vDest, pAI->GetLastNavMeshPoly(), pAI->GetRadius() ) )
	{
		pAI->GetAIBlackBoard()->SetBBInterpolateMovementHeight( true );
		pAI->GetAIBlackBoard()->SetBBScaleMovement( false );
		return;
	}

	// Turn on movement interpolation.

	pAI->GetAIBlackBoard()->SetBBInterpolateMovementHeight( true );
	pAI->GetAIBlackBoard()->SetBBScaleMovement( false );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionGotoTarget3D::DeactivateAction
//
//	PURPOSE:	Deactivate action.
//
// ----------------------------------------------------------------------- //

void CAIActionGotoTarget3D::DeactivateAction( CAI* pAI )
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

