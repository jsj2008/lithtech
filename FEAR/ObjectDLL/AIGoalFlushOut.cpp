// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalFlushOut.cpp
//
// PURPOSE : AIGoalFlushOut class implementation
//
// CREATED : 4/11/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIGoalFlushOut.h"
#include "AI.h"
#include "AIDB.h"
#include "AITarget.h"
#include "AIGoalMgr.h"
#include "AIBlackBoard.h"
#include "AIWorkingMemory.h"


DEFINE_AI_FACTORY_CLASS_SPECIFIC( Goal, CAIGoalFlushOut, kGoal_FlushOut );


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalFlushOut::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAIGoalFlushOut::CAIGoalFlushOut()
{
}

CAIGoalFlushOut::~CAIGoalFlushOut()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalFlushOut::CalculateGoalRelevance
//
//	PURPOSE:	Calculate the current goal relevance.
//
// ----------------------------------------------------------------------- //

void CAIGoalFlushOut::CalculateGoalRelevance()
{
	// AI is incapacitated.

	if( m_pAI->GetAIBlackBoard()->GetBBIncapacitated() )
	{
		m_fGoalRelevance = 0.f;
		return;
	}

	// Continue running until goal is satisfied.

	if( m_pAI->GetGoalMgr()->IsCurGoal( this ) )
	{
		m_fGoalRelevance = m_pGoalRecord->fIntrinsicRelevance;
		return;
	}

	// AI is aware of an enemy.
	// Enemy has gone out of view.
	// Enemy was seen within the last 5 seconds.

	if( m_pAI->HasTarget( kTarget_Character ) &&
		( m_pAI->GetAIBlackBoard()->GetBBTargetVisibilityConfidence() < 0.75f ) &&
		( g_pLTServer->GetTime() - m_pAI->GetAIBlackBoard()->GetBBTargetLastVisibleTime() < 5.f ) )
	{
		m_fGoalRelevance = m_pGoalRecord->fIntrinsicRelevance;
		return;
	}

	// No enemy present.

	m_fGoalRelevance = 0.f;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalFlushOut::ActivateGoal
//
//	PURPOSE:	Activate the goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalFlushOut::ActivateGoal()
{
	super::ActivateGoal();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalFlushOut::SetWSSatisfaction
//
//	PURPOSE:	Set the WorldState satisfaction conditions.
//
// ----------------------------------------------------------------------- //

void CAIGoalFlushOut::SetWSSatisfaction( CAIWorldState& WorldState )
{
	WorldState.SetWSProp( kWSK_TargetIsFlushedOut, m_pAI->m_hObject, kWST_bool, true );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalFlushOut::IsWSSatisfied
//
//	PURPOSE:	Return true if the world state satisfies the goal.
//
// ----------------------------------------------------------------------- //

bool CAIGoalFlushOut::IsWSSatisfied( CAIWorldState* pwsWorldState )
{
	SAIWORLDSTATE_PROP* pProp = pwsWorldState->GetWSProp( kWSK_TargetIsFlushedOut, m_pAI->m_hObject );
	if( pProp && ( pProp->bWSValue ) )
	{
		return true;
	}

	return false;
}

