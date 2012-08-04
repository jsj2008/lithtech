// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalRetreatLimited.cpp
//
// PURPOSE : AIGoalRetreat class implementation
//
// CREATED : 4/24/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIGoalRetreatLimited.h"
#include "AIWorkingMemoryCentral.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Goal, CAIGoalRetreatLimited, kGoal_RetreatLimited );

#define RETREAT_DELAY	30.f


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalRetreatLimited::CalculateGoalRelevance
//
//	PURPOSE:	Calculate the current goal relevance.
//
// ----------------------------------------------------------------------- //

void CAIGoalRetreatLimited::CalculateGoalRelevance()
{
	// Bail if taking damage.

	if( IsGoalInProgress() )
	{
		// Goal is relevant if we have been damaged recently.

		CAIWMFact factQuery;
		factQuery.SetFactType(kFact_Damage);
		CAIWMFact* pFact = m_pAI->GetAIWorkingMemory()->FindWMFact( factQuery );
		if( pFact && 
			DidDamage( m_pAI, pFact ) &&
			( pFact->GetUpdateTime() > m_fActivationTime ) )
		{
			m_fGoalRelevance = 0.f;
			return;
		}
	}

/***
	// Someone else is retreating.  Only one AI may retreat at a time.

	CAIWMFact factQuery;
	factQuery.SetFactType( kFact_Knowledge );
	factQuery.SetKnowledgeType( kKnowledge_Retreating );
	CAIWMFact* pFact = g_pAIWorkingMemoryCentral->FindWMFact(factQuery);
	if( pFact )
	{
		// Clear records of dead AI.

		if( IsDeadAI( pFact->GetSourceObject() ) )
		{
			g_pAIWorkingMemoryCentral->ClearWMFacts( factQuery );
		}

		// Retreating AI is not me.

		else if( pFact->GetSourceObject() != m_pAI->m_hObject )
		{
			m_fGoalRelevance = 0.f;
			return;
		}
	}

	// Someone else has retreated too recently.

	factQuery.SetFactType( kFact_Knowledge );
	factQuery.SetKnowledgeType( kKnowledge_NextRetreatTime );
	pFact = g_pAIWorkingMemoryCentral->FindWMFact(factQuery);
	if( pFact && ( pFact->GetTime() > g_pLTServer->GetTime() ) )
	{
		m_fGoalRelevance = 0.f;
		return;
	}
***/

	// Default behavior.

	super::CalculateGoalRelevance();
}

/*****

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalRetreatLimited::ActivateGoal
//
//	PURPOSE:	Activate the goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalRetreatLimited::ActivateGoal()
{
	super::ActivateGoal();

	// Record who is retreating.

	CAIWMFact* pFact = g_pAIWorkingMemoryCentral->CreateWMFact( kFact_Knowledge );
	if( pFact )
	{
		pFact->SetKnowledgeType( kKnowledge_NextRetreatTime, 1.f );
		pFact->SetSourceObject( m_pAI->m_hObject, 1.f );
	}
}
*****/

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalRetreatLimited::DeactivateGoal
//
//	PURPOSE:	Deactivate the goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalRetreatLimited::DeactivateGoal()
{
	super::DeactivateGoal();

	// Always remove the desire to retreat.

	CAIWMFact factQuery;
	factQuery.SetFactType(kFact_Desire);
	factQuery.SetDesireType(kDesire_Retreat);
	m_pAI->GetAIWorkingMemory()->ClearWMFacts( factQuery );

	/****

	// Remove knowledge of retreating AI.

	CAIWMFact factQuery;
	factQuery.SetFactType( kFact_Knowledge );
	factQuery.SetKnowledgeType( kKnowledge_Retreating );
	g_pAIWorkingMemoryCentral->ClearWMFact( factQuery );

	// Set next retreat time.

	factQuery.SetFactType( kFact_Knowledge );
	factQuery.SetKnowledgeType( kKnowledge_NextRetreatTime );
	CAIWMFact* pFact = g_pAIWorkingMemoryCentral->FindWMFact(factQuery);
	if( !pFact )
	{
		pFact = g_pAIWorkingMemoryCentral->CreateWMFact( kFact_Knowledge );
		pFact->SetKnowledgeType( kKnowledge_NextRetreatTime, 1.f );
	}
	pFact->SetTime( g_pLTServer->GetTime() + RETREAT_DELAY, 1.f );

	****/
}


