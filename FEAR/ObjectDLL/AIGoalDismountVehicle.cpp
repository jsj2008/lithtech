// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalDismountVehicle.cpp
//
// PURPOSE : AIGoalDismountVehicle class implementation
//
// CREATED : 12/29/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIGoalDismountVehicle.h"
#include "AI.h"
#include "AIDB.h"
#include "AIBlackBoard.h"


DEFINE_AI_FACTORY_CLASS_SPECIFIC( Goal, CAIGoalDismountVehicle, kGoal_DismountVehicle );


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalDismountVehicle::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAIGoalDismountVehicle::CAIGoalDismountVehicle()
{
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIGoalDismountVehicle::Save/Load
//              
//	PURPOSE:	Handle saving and restoring the CAIGoalGoto
//              
//----------------------------------------------------------------------------

void CAIGoalDismountVehicle::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);
}

void CAIGoalDismountVehicle::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalDismountVehicle::CalculateGoalRelevance
//
//	PURPOSE:	Calculate the current goal relevance.
//
// ----------------------------------------------------------------------- //

void CAIGoalDismountVehicle::CalculateGoalRelevance()
{
	// Goal is not relevant if AI is not riding anything.

	SAIWORLDSTATE_PROP* pProp = m_pAI->GetAIWorldState()->GetWSProp( kWSK_RidingVehicle, m_pAI->m_hObject );
	if( ( !pProp ) ||
		( pProp->eAnimPropWSValue == kAP_None ) )
	{
		m_fGoalRelevance = 0.f;
		return;
	}

	// Bail if we have no Dismount task.

	CAIWMFact factQuery;
	factQuery.SetFactType( kFact_Task );
	factQuery.SetTaskType( kTask_DismountVehicle );
	CAIWMFact* pFact = m_pAI->GetAIWorkingMemory()->FindWMFact( factQuery );
	if( pFact && ( pFact->GetConfidence( CAIWMFact::kFactMask_TaskType ) == 1.f ) )
	{
		m_fGoalRelevance = m_pGoalRecord->fIntrinsicRelevance;
		return;
	}

	// There is no Dismount task.

	m_fGoalRelevance = 0.f;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalDismountVehicle::ActivateGoal
//
//	PURPOSE:	Activate the goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalDismountVehicle::ActivateGoal()
{
	super::ActivateGoal();

	m_pAI->GetAIBlackBoard()->SetBBTaskStatus( kTaskStatus_Set );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalDismountVehicle::SetWSSatisfaction
//
//	PURPOSE:	Set the WorldState satisfaction conditions.
//
// ----------------------------------------------------------------------- //

void CAIGoalDismountVehicle::SetWSSatisfaction( CAIWorldState& WorldState )
{
	// Set satisfaction properties.

	WorldState.SetWSProp( kWSK_RidingVehicle, m_pAI->m_hObject, kWST_EnumAnimProp, kAP_None );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalDismountVehicle::IsWSSatisfied
//
//	PURPOSE:	Return true if the world state satisfies the goal.
//
// ----------------------------------------------------------------------- //

bool CAIGoalDismountVehicle::IsWSSatisfied( CAIWorldState* pwsWorldState )
{
	// Goal is satisfied if we are not riding anything.

	SAIWORLDSTATE_PROP* pProp = pwsWorldState->GetWSProp( kWSK_RidingVehicle, m_pAI->m_hObject );
	if( pProp && ( pProp->eAnimPropWSValue == kAP_None ) )
	{
		return true;
	}

	// We are riding something.

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalDismountVehicle::UpdateTaskStatus
//
//	PURPOSE:	Update the current status of the task.
//
// ----------------------------------------------------------------------- //

void CAIGoalDismountVehicle::UpdateTaskStatus()
{
	if( IsWSSatisfied( m_pAI->GetAIWorldState() ) )
	{
		m_pAI->GetAIBlackBoard()->SetBBTaskStatus( kTaskStatus_Done );

		CAIWMFact factQuery;
		factQuery.SetTaskType( kTask_DismountVehicle );
		m_pAI->GetAIWorkingMemory()->ClearWMFact( factQuery );
	}
}


