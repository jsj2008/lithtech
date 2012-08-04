// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalSearchLostTarget.cpp
//
// PURPOSE : AIGoalSearchLostTarget class implementation
//
// CREATED : 01/18/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIGoalSearchLostTarget.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Goal, CAIGoalSearchLostTarget, kGoal_SearchLostTarget );


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalSearchLostTarget::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAIGoalSearchLostTarget::CAIGoalSearchLostTarget()
{
}

CAIGoalSearchLostTarget::~CAIGoalSearchLostTarget()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalSearchLostTarget::CalculateGoalRelevance
//
//	PURPOSE:	Calculate the current goal relevance.
//
// ----------------------------------------------------------------------- //

void CAIGoalSearchLostTarget::CalculateGoalRelevance()
{
	// Only search from the target's lost position 
	// if not currently targeting anything.

	if( m_pAI->HasTarget( kTarget_All ) )
	{
		m_fGoalRelevance = 0.f;
		return;
	}

	// Goal is relevant if there is a Search task, and the
	// task does not specify a node.

	CAIWMFact factQuery;
	factQuery.SetFactType(kFact_Task);
	factQuery.SetTaskType(kTask_Search);
	CAIWMFact* pFact = m_pAI->GetAIWorkingMemory()->FindWMFact( factQuery );
	if( pFact && 
		( pFact->GetConfidence( CAIWMFact::kFactMask_TaskType ) == 1.f ) &&
		( !pFact->IsSet( CAIWMFact::kFactMask_TargetObject ) ) )
	{
		m_fGoalRelevance = m_pGoalRecord->fIntrinsicRelevance;
		return;
	}

	// No task or enemy present.

	m_fGoalRelevance = 0.f;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalSearchLostTarget::ActivateGoal
//
//	PURPOSE:	Activate the goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalSearchLostTarget::ActivateGoal()
{
	super::ActivateGoal();

	m_pAI->GetAIBlackBoard()->SetBBTaskStatus( kTaskStatus_Set );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalSearchLostTarget::SetWSSatisfaction
//
//	PURPOSE:	Set the WorldState satisfaction conditions.
//
// ----------------------------------------------------------------------- //

void CAIGoalSearchLostTarget::SetWSSatisfaction( CAIWorldState& WorldState )
{
	WorldState.SetWSProp( kWSK_AtTargetPos, m_pAI->m_hObject, kWST_bool, true );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalSearchLostTarget::IsWSSatisfied
//
//	PURPOSE:	Return true if the world state satisfies the goal.
//
// ----------------------------------------------------------------------- //

bool CAIGoalSearchLostTarget::IsWSSatisfied( CAIWorldState* pwsWorldState )
{
	SAIWORLDSTATE_PROP* pProp = pwsWorldState->GetWSProp( kWSK_AtTargetPos, m_pAI->m_hObject );
	if( !pProp || ( pProp && !pProp->bWSValue ) )
	{
		return false;
	}

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalSearchLostTarget::UpdateTaskStatus
//
//	PURPOSE:	Update the current status of the task.
//
// ----------------------------------------------------------------------- //

void CAIGoalSearchLostTarget::UpdateTaskStatus()
{
	// Clear completed task.

	if( IsWSSatisfied( m_pAI->GetAIWorldState() ) )
	{
		m_pAI->GetAIBlackBoard()->SetBBTaskStatus( kTaskStatus_Done );

		CAIWMFact factQuery;
		factQuery.SetFactType(kFact_Task);
		factQuery.SetTaskType(kTask_Search);
		m_pAI->GetAIWorkingMemory()->ClearWMFact( factQuery );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalSearchLostTarget::HandleBuildPlanFailure
//
//	PURPOSE:	Handle plan construction failing to find a valid plan to
//				accomplish this goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalSearchLostTarget::HandleBuildPlanFailure()
{
	super::HandleBuildPlanFailure();

	m_pAI->GetAIBlackBoard()->SetBBTaskStatus( kTaskStatus_Done );

	CAIWMFact factQuery;
	factQuery.SetFactType(kFact_Task);
	factQuery.SetTaskType(kTask_Search);
	m_pAI->GetAIWorkingMemory()->ClearWMFact( factQuery );
}

