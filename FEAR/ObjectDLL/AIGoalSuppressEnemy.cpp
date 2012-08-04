// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalSuppressEnemy.cpp
//
// PURPOSE : AIGoalSuppressEnemy class implementation
//
// CREATED : 5/30/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIGoalSuppressEnemy.h"
#include "AI.h"
#include "AIDB.h"
#include "AITarget.h"
#include "AIBlackBoard.h"
#include "AIWorkingMemory.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Goal, CAIGoalSuppressEnemy, kGoal_SuppressEnemy );


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalSuppressEnemy::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAIGoalSuppressEnemy::CAIGoalSuppressEnemy()
{
}

CAIGoalSuppressEnemy::~CAIGoalSuppressEnemy()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalSuppressEnemy::CalculateGoalRelevance
//
//	PURPOSE:	Calculate the current goal relevance.
//
// ----------------------------------------------------------------------- //

void CAIGoalSuppressEnemy::CalculateGoalRelevance()
{
	// Goal is relevant if there is a SuppressionFire task,
	// and AI is aware of an enemy.

	CAIWMFact factQuery;
	factQuery.SetFactType(kFact_Task);
	factQuery.SetTaskType(kTask_SuppressionFire);
	CAIWMFact* pFact = m_pAI->GetAIWorkingMemory()->FindWMFact( factQuery );
	if( pFact && 
		( pFact->GetConfidence( CAIWMFact::kFactMask_TaskType ) == 1.f ) &&
		( m_pAI->HasTarget( kTarget_Character ) ) )
	{
		m_pAI->GetAIBlackBoard()->SetBBTaskStatus( kTaskStatus_Set );
		m_fGoalRelevance = m_pGoalRecord->fIntrinsicRelevance;
		return;
	}

	// No task or enemy present.

	m_fGoalRelevance = 0.f;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalSuppressEnemy::ActivateGoal
//
//	PURPOSE:	Activate the goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalSuppressEnemy::ActivateGoal()
{
	super::ActivateGoal();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalSuppressEnemy::UpdateGoal
//
//	PURPOSE:	Return true if goal should keep running.
//
// ----------------------------------------------------------------------- //

bool CAIGoalSuppressEnemy::UpdateGoal()
{
	// AI has been damaged since this goal activated.

	CAIWMFact factQuery;
	factQuery.SetFactType(kFact_Damage);
	CAIWMFact* pFact = m_pAI->GetAIWorkingMemory()->FindWMFact( factQuery );
	if( pFact && ( pFact->GetUpdateTime() > m_fActivationTime ) )
	{
		// Clear the suppression fire task.

		CAIWMFact factQuery;
		factQuery.SetFactType(kFact_Task);
		factQuery.SetTaskType(kTask_SuppressionFire);
		pFact = m_pAI->GetAIWorkingMemory()->FindWMFact( factQuery );
		if( pFact )
		{
			CAIWMFact RemovefactQuery;
			RemovefactQuery.SetFactType(kFact_Task);
			m_pAI->GetAIWorkingMemory()->ClearWMFacts( RemovefactQuery );
		}

		return false;
	}

	// Default behavior.

	return super::UpdateGoal();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalSuppressEnemy::SetWSSatisfaction
//
//	PURPOSE:	Set the WorldState satisfaction conditions.
//
// ----------------------------------------------------------------------- //

void CAIGoalSuppressEnemy::SetWSSatisfaction( CAIWorldState& WorldState )
{
	WorldState.SetWSProp( kWSK_TargetIsSuppressed, m_pAI->m_hObject, kWST_bool, true );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalSuppressEnemy::IsWSSatisfied
//
//	PURPOSE:	Return true if the world state satisfies the goal.
//
// ----------------------------------------------------------------------- //

bool CAIGoalSuppressEnemy::IsWSSatisfied( CAIWorldState* pwsWorldState )
{
	SAIWORLDSTATE_PROP* pProp = pwsWorldState->GetWSProp( kWSK_TargetIsSuppressed, m_pAI->m_hObject );
	if( pProp && ( pProp->bWSValue ) )
	{
		return true;
	}

	return false;
}

