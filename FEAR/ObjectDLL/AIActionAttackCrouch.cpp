// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionAttackCrouch.cpp
//
// PURPOSE : AIActionAttackCrouch class implementation
//
// CREATED : 3/18/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIActionAttackCrouch.h"
#include "AI.h"
#include "AIDB.h"
#include "AITarget.h"
#include "AIBlackBoard.h"
#include "AnimationContext.h"
#include "NodeTrackerContext.h"
#include "AIPathMgrNavMesh.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionAttackCrouch, kAct_AttackCrouch );


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionAttack::ValidateContextPreconditions
//
//	PURPOSE:	Return true if real-time preconditions are valid.
//
// ----------------------------------------------------------------------- //

bool CAIActionAttackCrouch::ValidateContextPreconditions( CAI* pAI, CAIWorldState& wsWorldStateGoal, bool bIsPlanning )
{
	if( !super::ValidateContextPreconditions( pAI, wsWorldStateGoal, bIsPlanning ) )
	{
		return false;
	}

	// Do not attack crouch if AI is at a node.

	SAIWORLDSTATE_PROP* pProp = pAI->GetAIWorldState()->GetWSProp( kWSK_AtNode, pAI->m_hObject );
	if( pProp && pProp->hWSValue )
	{
		return false;
	}

	// Target is far enough away.

	float fThreshold = 2.f * g_pAIDB->GetAIConstantsRecord()->fThreatTooCloseDistance;
	if( pAI->GetTarget()->GetTargetDistSqr() < fThreshold * fThreshold )
	{
		return false;
	}

	// No straight path to the target (geometry may obstruct the crouch, so don't do it).

	if( !g_pAIPathMgrNavMesh->StraightPathExists( pAI, pAI->GetCharTypeMask(), pAI->GetPosition(), pAI->GetAIBlackBoard()->GetBBTargetPosition(), pAI->GetAIBlackBoard()->GetBBTargetReachableNavMeshPoly(), pAI->GetRadius() ) )
	{
		return false;
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionAttackCrouch::ActivateAction
//
//	PURPOSE:	Activate action.
//
// ----------------------------------------------------------------------- //

void CAIActionAttackCrouch::ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal )
{
	super::ActivateAction( pAI, wsWorldStateGoal );

	// Torso tracking.

	pAI->GetAIBlackBoard()->SetBBTargetTrackerFlags( kTrackerFlag_AimAt );
	pAI->GetAIBlackBoard()->SetBBFaceTarget( true );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionAttackCrouch::SetAttackAnimProps
//
//	PURPOSE:	Set animation props.
//
// ----------------------------------------------------------------------- //

void CAIActionAttackCrouch::SetAttackAnimProps( CAI* pAI, CAnimationProps* pProps )
{
	// Sanity check.

	if( !( pAI && pProps ) )
	{
		return;
	}

	super::SetAttackAnimProps( pAI, pProps );

	pProps->Set( kAPG_Posture, kAP_POS_Crouch );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionAttackCrouch::ValidateAction
//
//	PURPOSE:	Return true if action is still valid.
//
// ----------------------------------------------------------------------- //

bool CAIActionAttackCrouch::ValidateAction( CAI* pAI )
{
	if( !super::ValidateAction( pAI ) )
	{
		return false;
	}

	// Target is too close.

	if( pAI->GetTarget()->GetTargetDistSqr() < g_pAIDB->GetAIConstantsRecord()->fThreatTooCloseDistanceSqr )
	{
		return false;
	}

	return true;
}


