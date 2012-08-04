// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalFinishBlockSuccess.cpp
//
// PURPOSE : 
//
// CREATED : 11/09/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIGoalFinishBlockSuccess.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Goal, CAIGoalFinishBlockSuccess, kGoal_FinishBlockSuccess );

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalFinishBlockSuccess::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAIGoalFinishBlockSuccess::CAIGoalFinishBlockSuccess()
{
}

CAIGoalFinishBlockSuccess::~CAIGoalFinishBlockSuccess()
{
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIGoalFinishBlockSuccess::CalculateGoalRelevance
//              
//	PURPOSE:	Calculate the current goal relevance.
//              
//----------------------------------------------------------------------------

void CAIGoalFinishBlockSuccess::CalculateGoalRelevance()
{
	// Goal is not relevant if the AI did not just fail a block

	CAIWMFact queryFact;
	queryFact.SetFactType( kFact_Knowledge );
	queryFact.SetKnowledgeType( kKnowledge_MeleeBlockSuccess );
	CAIWMFact* pBlockedFinishDesireFact = m_pAI->GetAIWorkingMemory()->FindWMFact( queryFact );
	if ( !pBlockedFinishDesireFact )
	{
		m_fGoalRelevance = 0.0f;
		return;
	}

	// Goal is relevant.

	m_fGoalRelevance = m_pGoalRecord->fIntrinsicRelevance;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIGoalFinishBlockSuccess::ActivateGoal
//              
//	PURPOSE:	Performs any additional actions on activation of the goal. In
//				articular, removes the fact which queues this goal to prevent
//				accidental reactivation.
//              
//----------------------------------------------------------------------------

void CAIGoalFinishBlockSuccess::ActivateGoal()
{
	super::ActivateGoal();

	// Remove the fact.

	CAIWMFact queryFact;
	queryFact.SetFactType( kFact_Knowledge );
	queryFact.SetKnowledgeType( kKnowledge_MeleeBlockSuccess );
	m_pAI->GetAIWorkingMemory()->ClearWMFact( queryFact );
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIGoalFinishBlockSuccess::SetWSSatisfaction
//              
//	PURPOSE:	Set the WorldState satisfaction conditions.
//              
//----------------------------------------------------------------------------

void CAIGoalFinishBlockSuccess::SetWSSatisfaction( CAIWorldState& WorldState )
{
	WorldState.SetWSProp( kWSK_ReactedToWorldStateEvent, m_pAI->m_hObject, kWST_ENUM_AIWorldStateEvent, kWSE_MeleeBlockSuccess );
}
