// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalIdle.cpp
//
// PURPOSE : AIGoalIdle class implementation
//
// CREATED : 1/28/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIGoalIdle.h"
#include "AI.h"
#include "AIDB.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Goal, CAIGoalIdle, kGoal_Idle );


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalIdle::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAIGoalIdle::CAIGoalIdle()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalIdle::CalculateGoalRelevance
//
//	PURPOSE:	Calculate the current goal relevance.
//
// ----------------------------------------------------------------------- //

void CAIGoalIdle::CalculateGoalRelevance()
{
	// The idle goal has a constant, very low relevance.
	// It should always be active if nothing else is.

	m_fGoalRelevance = m_pGoalRecord->fIntrinsicRelevance;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalIdle::SetWSSatisfaction
//
//	PURPOSE:	Set the WorldState satisfaction conditions.
//
// ----------------------------------------------------------------------- //

void CAIGoalIdle::SetWSSatisfaction( CAIWorldState& WorldState )
{
	WorldState.SetWSProp( kWSK_Idling, m_pAI->m_hObject, kWST_bool, true );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalIdle::IsWSSatisfied
//
//	PURPOSE:	Return true if the world state satisfies the goal.
//
// ----------------------------------------------------------------------- //

bool CAIGoalIdle::IsWSSatisfied( CAIWorldState* pwsWorldState )
{
	// The idle goal is never satisfied in real-time,
	// but the planner should always consider it satisfied.

	return false;
}
