// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalCharge.cpp
//
// PURPOSE : AIGoalCharge class implementation
//
// CREATED  08/18/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIGoalCharge.h"
#include "AI.h"
#include "AIGoalMgr.h"
#include "AIBlackBoard.h"
#include "AIWorkingMemoryCentral.h"
#include "AIPathMgrNavMesh.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Goal, CAIGoalCharge, kGoal_Charge );


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalCharge::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAIGoalCharge::CAIGoalCharge()
{
}

CAIGoalCharge::~CAIGoalCharge()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalCharge::CalculateGoalRelevance
//
//	PURPOSE:	Calculate the current goal relevance.
//
// ----------------------------------------------------------------------- //

void CAIGoalCharge::CalculateGoalRelevance()
{
	// AI doesn't have a target.

	if( !m_pAI->HasTarget( kTarget_Character ) )
	{
		m_fGoalRelevance = 0.f;
		return;
	}

	// No relevance if AI does not have a deisre to lunge.
	// The desire indicates the max range of the lunge.

	CAIWMFact factDesireQuery;
	factDesireQuery.SetFactType( kFact_Desire );
	factDesireQuery.SetDesireType( kDesire_Lunge );
	CAIWMFact* pDesireFact = m_pAI->GetAIWorkingMemory()->FindWMFact( factDesireQuery );
	if( !pDesireFact )
	{
		m_fGoalRelevance = 0.f;
		return;
	}

	// Someone has lunged too recently.

	CAIWMFact factQuery;
	factQuery.SetFactType( kFact_Knowledge );
	factQuery.SetKnowledgeType( kKnowledge_NextLungeTime );
	CAIWMFact* pFact = g_pAIWorkingMemoryCentral->FindWMFact(factQuery);
	if( pFact && ( pFact->GetTime() > g_pLTServer->GetTime() ) )
	{
		m_fGoalRelevance = 0.f;
		return;
	}

	// Target must be in range.

	LTVector vTarget = m_pAI->GetAIBlackBoard()->GetBBTargetPosition();
	float fDistSqr = vTarget.DistSqr( m_pAI->GetPosition() );

	// Target too far.

	float fMaxDist = pDesireFact->GetRadius();
	if( fDistSqr > fMaxDist * fMaxDist )
	{
		m_fGoalRelevance = 0.f;
		return;
	}

	// No straight path to the target.

	if( !g_pAIPathMgrNavMesh->StraightPathExists( m_pAI, m_pAI->GetCharTypeMask(), m_pAI->GetPosition(), vTarget, m_pAI->GetAIBlackBoard()->GetBBTargetReachableNavMeshPoly(), 0.f ) )
	{
		m_fGoalRelevance = 0.f;
		return;
	}

	// Default handling from KillEnemy.

	super::CalculateGoalRelevance();
}

