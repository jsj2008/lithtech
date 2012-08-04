// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalSearchFollow.cpp
//
// PURPOSE : AIGoalSearchFollow class implementation
//
// CREATED : 9/21/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIGoalSearchFollow.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Goal, CAIGoalSearchFollow, kGoal_SearchFollow );


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalSearchFollow::CalculateGoalRelevance
//
//	PURPOSE:	Calculate the current goal relevance.
//
// ----------------------------------------------------------------------- //

void CAIGoalSearchFollow::CalculateGoalRelevance()
{
	super::CalculateGoalRelevance();

	// Bail if default checks failed.

	if( m_fGoalRelevance == 0.f )
	{
		return;
	}

	//
	// Determine if leader is searching.
	//

	// Bail if we have no Follow task.

	CAIWMFact factQuery;
	factQuery.SetFactType( kFact_Task );
	factQuery.SetTaskType( kTask_Follow );
	CAIWMFact* pFact = m_pAI->GetAIWorkingMemory()->FindWMFact( factQuery );
	if( !pFact )
	{
		m_fGoalRelevance = 0.f;
		return;
	}

	// Leader must be an AI.

	HOBJECT hLeader = pFact->GetSourceObject();
	if( !IsAI( hLeader ) )
	{
		m_fGoalRelevance = 0.f;
		return;
	}
	CAI* pAI = (CAI*)g_pLTServer->HandleToObject( hLeader );

	// Fail if leader is not searching.

	factQuery.SetTaskType( kTask_Search );
	pFact = pAI->GetAIWorkingMemory()->FindWMFact( factQuery );
	if( !( pFact && ( pFact->GetConfidence( CAIWMFact::kFactMask_TaskType ) == 1.f ) ) )
	{
		factQuery.SetTaskType( kTask_Alert );
		pFact = pAI->GetAIWorkingMemory()->FindWMFact( factQuery );
		if( !( pFact && ( pFact->GetConfidence( CAIWMFact::kFactMask_TaskType ) == 1.f ) ) )
		{
			m_fGoalRelevance = 0.f;
			return;
		}
	}
}

