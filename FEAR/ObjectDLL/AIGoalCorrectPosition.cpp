// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalCorrectPosition.cpp
//
// PURPOSE : Implements the 'correct position' desire.
//
// CREATED : 4/02/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIGoalCorrectPosition.h"
#include "AI.h"
#include "AIDB.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Goal, CAIGoalCorrectPosition, kGoal_CorrectPosition );


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalCorrectPosition::CAIGoalCorrectPosition()
//
//	PURPOSE:	Handle Construction/Destruction of a CAIGoalCorrectPosition 
//				instance
//
// ----------------------------------------------------------------------- //
CAIGoalCorrectPosition::CAIGoalCorrectPosition()
{
}

CAIGoalCorrectPosition::~CAIGoalCorrectPosition()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalCorrectPosition::CalculateGoalRelevance
//
//	PURPOSE:	Calculate the current goal relevance.
//
// ----------------------------------------------------------------------- //

void CAIGoalCorrectPosition::CalculateGoalRelevance()
{
	// Do not correct the AIs position if the position is currently valid.

	SAIWORLDSTATE_PROP* pProp = m_pAI->GetAIWorldState()->GetWSProp( kWSK_PositionIsValid, m_pAI->m_hObject );
	if (pProp && pProp->bWSValue)
	{
		m_fGoalRelevance = 0.f;
		return;
	}

	// Default calculation.

	m_fGoalRelevance = m_pGoalRecord->fIntrinsicRelevance;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalCorrectPosition::SetWSSatisfaction
//
//	PURPOSE:	Set the WorldState satisfaction conditions.
//
// ----------------------------------------------------------------------- //

void CAIGoalCorrectPosition::SetWSSatisfaction( CAIWorldState& WorldState )
{
	super::SetWSSatisfaction(WorldState);
	
	// Set satisfaction properties.

	WorldState.SetWSProp( kWSK_PositionIsValid, m_pAI->m_hObject, kWST_bool, true );
}
