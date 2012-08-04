// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalFinishBlockFailure.cpp
//
// PURPOSE : 
//
// CREATED : 11/09/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIGoalFinishBlockFailure.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Goal, CAIGoalFinishBlockFailure, kGoal_FinishBlockFailure );

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalFinishBlockFailure::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAIGoalFinishBlockFailure::CAIGoalFinishBlockFailure()
{
}

CAIGoalFinishBlockFailure::~CAIGoalFinishBlockFailure()
{
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIGoalFinishBlockFailure::CalculateGoalRelevance
//              
//	PURPOSE:	Calculate the current goal relevance.
//              
//----------------------------------------------------------------------------

void CAIGoalFinishBlockFailure::CalculateGoalRelevance()
{
	// Goal is not relevant if the AI did not just fail a block

	CAIWMFact queryFact;
	queryFact.SetFactType( kFact_Knowledge );
	queryFact.SetKnowledgeType( kKnowledge_MeleeBlockFailure );
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

void CAIGoalFinishBlockFailure::ActivateGoal()
{
	super::ActivateGoal();

	// Remove the fact.

	CAIWMFact queryFact;
	queryFact.SetFactType( kFact_Knowledge );
	queryFact.SetKnowledgeType( kKnowledge_MeleeBlockFailure );
	m_pAI->GetAIWorkingMemory()->ClearWMFact( queryFact );
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIGoalFinishBlockFailure::SetWSSatisfaction
//              
//	PURPOSE:	Set the WorldState satisfaction conditions.
//              
//----------------------------------------------------------------------------

void CAIGoalFinishBlockFailure::SetWSSatisfaction( CAIWorldState& WorldState )
{
	WorldState.SetWSProp( kWSK_ReactedToWorldStateEvent, m_pAI->m_hObject, kWST_ENUM_AIWorldStateEvent, kWSE_MeleeBlockFailure );
}
