// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionLopeToTargetUncloaked.cpp
//
// PURPOSE : AIActionLopeToTargetUncloaked class implementation
//
// CREATED : 5/03/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIActionLopeToTargetUncloaked.h"
#include "AIStateGoto.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionLopeToTargetUncloaked, kAct_LopeToTargetUncloaked );


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionLopeToTargetUncloaked::Con/destructor
//
//	PURPOSE:	Con/destructor
//
// ----------------------------------------------------------------------- //

CAIActionLopeToTargetUncloaked::CAIActionLopeToTargetUncloaked()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionLopeToTargetUncloaked::InitAction
//
//	PURPOSE:	Handle initializing the AIAction, setting the 
//				ActionRecord to use, as well as preconditions and effects.
//
// ----------------------------------------------------------------------- //

void CAIActionLopeToTargetUncloaked::InitAction( AIDB_ActionRecord* pActionRecord )
{
	super::InitAction( pActionRecord );

	// No preconditions.

	// Set effects.
	// At the target's pos.

	m_wsWorldStateEffects.SetWSProp( kWSK_AtTargetPos, NULL, kWST_bool, true );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionLopeToTargetUncloaked::ValidateContextPreconditions
//
//	PURPOSE:	Return true if real-time preconditions are valid.
//
// ----------------------------------------------------------------------- //

bool CAIActionLopeToTargetUncloaked::ValidateContextPreconditions( CAI* pAI, CAIWorldState& wsWorldStateGoal, bool bIsPlanning )
{
	if( !super::ValidateContextPreconditions( pAI, wsWorldStateGoal, bIsPlanning ) )
	{
		return false;
	}

	// Only check path if planning.

	if( !bIsPlanning )
	{
		return true;
	}

	// Target is too close.

	LTVector vTarget = pAI->GetAIBlackBoard()->GetBBTargetReachableNavMeshPosition();
	if( vTarget.DistSqr( pAI->GetPosition() ) < 650.f * 650.f )
	{
		return false;
	}

	// No straight path exists to the target.

	LTVector vDir = vTarget - pAI->GetPosition();
	vDir.Normalize();
	LTVector vDest = pAI->GetPosition() + ( vDir * 650.f );
	if( !g_pAIPathMgrNavMesh->StraightPathExists( pAI, pAI->GetCharTypeMask(), pAI->GetPosition(), vDest, pAI->GetAIBlackBoard()->GetBBTargetReachableNavMeshPoly(), pAI->GetRadius() ) )
	{
		return false;
	}

	// Lope!

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionLopeToTargetUncloaked::ActivateAction
//
//	PURPOSE:	Activate action.
//
// ----------------------------------------------------------------------- //

void CAIActionLopeToTargetUncloaked::ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal )
{
	super::ActivateAction( pAI, wsWorldStateGoal );

	if( !pAI->HasTarget( kTarget_All ) )
	{
		return;
	}

	// Sanity check.

	if( pAI->GetState()->GetStateClassType() != kState_Goto )
	{
		return;
	}

	// Set loping activity prop.

	CAIStateGoto* pGoto = (CAIStateGoto*)pAI->GetState();
	pGoto->SetActivityProp( kAP_ATVT_Loping );
	
	// This Actions does not Uncloak the AI.
	// It is assumed that the AI will uncloak thru a
	// DESIRE modelstring in the lunge animation.
	// This allows the AI to uncloak in mid-lunge.
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionLopeToTargetUncloaked::DeactivateAction
//
//	PURPOSE:	Deactivate action.
//
// ----------------------------------------------------------------------- //

void CAIActionLopeToTargetUncloaked::DeactivateAction( CAI* pAI )
{
	super::DeactivateAction( pAI );

	// Remove the desire to uncloak.

	CAIWMFact factQuery;
	factQuery.SetFactType(kFact_Desire);
	factQuery.SetDesireType(kDesire_Uncloak);
	pAI->GetAIWorkingMemory()->ClearWMFacts( factQuery );
}
