// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalLead.cpp
//
// PURPOSE : 
//
// CREATED : 4/07/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIGoalLead.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Goal, CAIGoalLead, kGoal_Lead );

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalLead::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAIGoalLead::CAIGoalLead()
{
}

CAIGoalLead::~CAIGoalLead()
{
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIGoalLead::Save/Load
//              
//	PURPOSE:	Handle saving and restoring the CAIGoalLead
//              
//----------------------------------------------------------------------------

void CAIGoalLead::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);

	LOAD_INT_CAST( m_ePendingLeadRequest, ENUM_FactID );
	LOAD_INT_CAST( m_eCurrentLeadRequest, ENUM_FactID );
}

void CAIGoalLead::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);

	SAVE_INT( m_ePendingLeadRequest );
	SAVE_INT( m_eCurrentLeadRequest );
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIGoalLead::CalculateGoalRelevance
//              
//	PURPOSE:	Calculate the current goal relevance.
//              
//----------------------------------------------------------------------------

void CAIGoalLead::CalculateGoalRelevance()
{
	// Not relevant unless the AI is targeting a follower.

	if ( false == m_pAI->HasTarget( kTarget_Follower ) )
	{
		m_fGoalRelevance = 0.0f;
		return;
	}

	// Not relevant if the AI goes not have a 'lead character' task

	CAIWMFact queryFact;
	queryFact.SetFactType( kFact_Task );
	queryFact.SetTaskType( kTask_LeadCharacter );
	CAIWMFact* pFact = m_pAI->GetAIWorkingMemory()->FindWMFact( queryFact );
	if ( NULL == pFact )
	{
		m_fGoalRelevance = 0.0f;
		return;
	}

	// Not relevant if either the destination or the character to be lead is 
	// dead/does not exist.

	if ( IsDeadCharacter( pFact->GetSourceObject() ) 
		|| NULL == pFact->GetTargetObject() )
	{
		m_fGoalRelevance = 0.0f;
		return;
	}

	// Goal is relevant.

	m_fGoalRelevance = m_pGoalRecord->fIntrinsicRelevance;
	m_ePendingLeadRequest = pFact->GetFactID();
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIGoalLead::ActivateGoal
//              
//	PURPOSE:	Activate the goal.
//              
//----------------------------------------------------------------------------

void CAIGoalLead::ActivateGoal()
{
	super::ActivateGoal();

	m_eCurrentLeadRequest = m_ePendingLeadRequest;
	m_ePendingLeadRequest = kFactID_Invalid;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIGoalLead::DeactivateGoal
//              
//	PURPOSE:	Deactivate the goal.
//              
//----------------------------------------------------------------------------

void CAIGoalLead::DeactivateGoal()
{
	super::DeactivateGoal();

	m_eCurrentLeadRequest = kFactID_Invalid;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIGoalLead::SetWSSatisfaction
//              
//	PURPOSE:	Set the WorldState satisfaction conditions.
//              
//----------------------------------------------------------------------------

void CAIGoalLead::SetWSSatisfaction( CAIWorldState& WorldState )
{
	CAIWMFact queryFact;
	queryFact.SetFactID( m_ePendingLeadRequest );
	CAIWMFact* pFact = m_pAI->GetAIWorkingMemory()->FindWMFact( queryFact );

	WorldState.SetWSProp( kWSK_AtNode, m_pAI->m_hObject, kWST_HOBJECT, pFact->GetTargetObject() );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalLead::UpdateTaskStatus
//
//	PURPOSE:	Update the current status of the task.
//
// ----------------------------------------------------------------------- //

void CAIGoalLead::UpdateTaskStatus()
{
	// Clear completed task.

	if( IsWSSatisfied( m_pAI->GetAIWorldState() ) )
	{
		CAIWMFact queryFact;
		queryFact.SetFactID( m_eCurrentLeadRequest );
		m_pAI->GetAIWorkingMemory()->ClearWMFacts( queryFact );

		m_eCurrentLeadRequest = kFactID_Invalid;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalLead::HandleBuildPlanFailure
//
//	PURPOSE:	Handle plan construction failing to find a valid plan to
//				accomplish this goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalLead::HandleBuildPlanFailure()
{
	super::HandleBuildPlanFailure();

	CAIWMFact queryFact;
	queryFact.SetFactID( m_ePendingLeadRequest );
	m_pAI->GetAIWorkingMemory()->ClearWMFacts( queryFact );

	m_ePendingLeadRequest = kFactID_Invalid;
}
