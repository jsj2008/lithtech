// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionAttackMeleeFromDistance.cpp
//
// PURPOSE : 
//
// CREATED : 11/03/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIActionAttackMeleeFromDistance.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionAttackMeleeFromDistance, kAct_AttackMeleeFromDistance );

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionAttackMeleeFromDistance::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAIActionAttackMeleeFromDistance::CAIActionAttackMeleeFromDistance()
{
	m_bDistanceClosingAttack = true;
}

CAIActionAttackMeleeFromDistance::~CAIActionAttackMeleeFromDistance()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionAttackMeleeFromDistance::ValidateContextPreconditions
//
//	PURPOSE:	Return true if real-time preconditions are valid.
//
// ----------------------------------------------------------------------- //

bool CAIActionAttackMeleeFromDistance::ValidateContextPreconditions( CAI* pAI, CAIWorldState& wsWorldStateGoal, bool bIsPlanning )
{
	if ( !super::ValidateContextPreconditions( pAI, wsWorldStateGoal, bIsPlanning ) )
	{
		return false;
	}

	// Do not allow this to activate if the current movement property is set, 
	// as this causes an ugly animation pop (requested by Frank)

	if ( pAI->GetAnimationContext()->IsPropSet(kAPG_Movement, kAP_MOV_Run) 
		|| pAI->GetAnimationContext()->IsPropSet(kAPG_Movement, kAP_MOV_Walk ) )
	{
		return false;
	}

	// Bail if Action does not have an existing SmartObject.

	AIDB_SmartObjectRecord* pSmartObjectRecord = g_pAIDB->GetAISmartObjectRecord( m_pActionRecord->eSmartObjectID );
	if( !pSmartObjectRecord )
	{
		return false;
	}

	LTVector vDistance2D = pAI->GetAIBlackBoard()->GetBBTargetPosition() - pAI->GetPosition();
	vDistance2D.y = 0;

	// Enemy is too close

	float flDistSqr = vDistance2D.MagSqr();
	if ( flDistSqr < LTSqr(pSmartObjectRecord->fMinDist)  )
	{
		return false;
	}

	// Enemy is too far

	if ( flDistSqr > LTSqr( pSmartObjectRecord->fMaxDist ) )
	{
		return false;
	}

	// Bail if there is not a straight path through the mesh to the enemy

	if( !g_pAIPathMgrNavMesh->StraightPathExists( pAI, pAI->GetCharTypeMask(), pAI->GetPosition(), pAI->GetAIBlackBoard()->GetBBTargetPosition(), pAI->GetAIBlackBoard()->GetBBTargetReachableNavMeshPoly(), pAI->GetRadius() ) )
	{
		return false;
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionAttackMeleeFromDistance::SetAttackAnimProps
//
//	PURPOSE:	Set animation props.
//
// ----------------------------------------------------------------------- //

void CAIActionAttackMeleeFromDistance::SetAttackAnimProps( CAI* pAI, CAnimationProps* pProps )
{
	super::SetAttackAnimProps( pAI, pProps );

	// Sanity check.

	if( !pProps )
	{
		return;
	}

	pProps->Set( kAPG_Action, kAP_ACT_AttackMeleeFromDistance );
}
