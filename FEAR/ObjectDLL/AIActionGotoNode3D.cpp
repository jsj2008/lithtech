// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionGotoNode3D.cpp
//
// PURPOSE : AIActionGotoNode3D class implementation
//
// CREATED : 9/01/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIActionGotoNode3D.h"
#include "AI.h"
#include "AIStateGoto.h"
#include "AIPathMgrNavMesh.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionGotoNode3D, kAct_GotoNode3D );


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionGotoNode3D::ActivateAction
//
//	PURPOSE:	Activate action.
//
// ----------------------------------------------------------------------- //

void CAIActionGotoNode3D::ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal )
{
	super::ActivateAction( pAI, wsWorldStateGoal );

	// Sanity check.

	if( !( pAI && pAI->GetState() && pAI->GetState()->GetStateClassType() == kState_Goto ) )
	{
		return;
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

	// Turn on movement scaling.

	pAI->GetAIBlackBoard()->SetBBInterpolateMovementHeight( false );
	pAI->GetAIBlackBoard()->SetBBScaleMovement( true );

	// Select and up or down animation.

	CAIStateGoto* pStateGoto = (CAIStateGoto*)( pAI->GetState() );
	if( pAI->GetPosition().y > vDest.y )
	{
		pStateGoto->SetActivityProp( kAP_ATVT_Descending );
	}
	else {
		pStateGoto->SetActivityProp( kAP_ATVT_Ascending );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionGotoNode3D::DeactivateAction
//
//	PURPOSE:	Deactivate action.
//
// ----------------------------------------------------------------------- //

void CAIActionGotoNode3D::DeactivateAction( CAI* pAI )
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

