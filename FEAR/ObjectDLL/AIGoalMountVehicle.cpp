// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalMountVehicle.cpp
//
// PURPOSE : AIGoalMountVehicle class implementation
//
// CREATED : 12/23/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIGoalMountVehicle.h"
#include "AI.h"
#include "AINode.h"
#include "AIBlackBoard.h"
#include "AIWorkingMemory.h"
#include "AIUtils.h"


DEFINE_AI_FACTORY_CLASS_SPECIFIC( Goal, CAIGoalMountVehicle, kGoal_MountVehicle );


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalMountVehicle::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAIGoalMountVehicle::CAIGoalMountVehicle()
{
	m_eTaskType = kTask_MountVehicle;
	m_bClearScriptedTaskIfThreatened = false;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIGoalMountVehicle::Save/Load
//              
//	PURPOSE:	Handle saving and restoring the CAIGoalGoto
//              
//----------------------------------------------------------------------------

void CAIGoalMountVehicle::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);
}

void CAIGoalMountVehicle::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalMountVehicle::CalculateGoalRelevance
//
//	PURPOSE:	Calculate the current goal relevance.
//
// ----------------------------------------------------------------------- //

void CAIGoalMountVehicle::CalculateGoalRelevance()
{
	// Goal is not relevant if AI is already riding something.

	SAIWORLDSTATE_PROP* pProp = m_pAI->GetAIWorldState()->GetWSProp( kWSK_RidingVehicle, m_pAI->m_hObject );
	if( pProp && ( pProp->eAnimPropWSValue != kAP_None ) )
	{
		m_fGoalRelevance = 0.f;
		return;
	}

	// Default behavior.

	super::CalculateGoalRelevance();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalMountVehicle::DeactivateGoal
//
//	PURPOSE:	Deactivate the goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalMountVehicle::DeactivateGoal()
{
	// Clear knowledge of the node once the AI has mounted the vehicle.

	AINodeSmartObject* pNodeVehicle = (AINodeSmartObject*)g_pLTServer->HandleToObject( m_NodeCurrent.hNode );
	pNodeVehicle->HandleAIDeparture( m_pAI );

	// Clear ALL MountVehicle tasks.

	CAIWMFact factQuery;
	factQuery.SetFactType( kFact_Task );
	factQuery.SetTaskType( kTask_MountVehicle );
	m_pAI->GetAIWorkingMemory()->ClearWMFacts( factQuery );
	m_NodeCurrent.eFactID = kFactID_Invalid;
	m_NodeCurrent.hNode = NULL;

	// Default behavior.

	super::DeactivateGoal();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalMountVehicle::SetWSSatisfaction
//
//	PURPOSE:	Set the WorldState satisfaction conditions.
//
// ----------------------------------------------------------------------- //

void CAIGoalMountVehicle::SetWSSatisfaction( CAIWorldState& WorldState )
{
	// Intentionally do NOT call super.
	// Set satisfaction properties.

	WorldState.SetWSProp( kWSK_UsingObject, m_pAI->m_hObject, kWST_HOBJECT, m_NodePending.hNode );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalMountVehicle::IsWSSatisfied
//
//	PURPOSE:	Return true if the world state satisfies the goal.
//
// ----------------------------------------------------------------------- //

bool CAIGoalMountVehicle::IsWSSatisfied( CAIWorldState* pwsWorldState )
{
	// Intentionally do NOT call super.
	// Goal is satisfied if we have used the node.

	SAIWORLDSTATE_PROP* pProp = pwsWorldState->GetWSProp( kWSK_UsingObject, m_pAI->m_hObject );
	if( pProp && pProp->hWSValue && ( pProp->hWSValue == m_NodeCurrent.hNode ) )
	{
		return true;
	}

	// Not used the node.

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalMountVehicle::UpdateTaskStatus
//
//	PURPOSE:	Update the current status of the task.
//
// ----------------------------------------------------------------------- //

void CAIGoalMountVehicle::UpdateTaskStatus()
{
	if( IsWSSatisfied( m_pAI->GetAIWorldState() ) )
	{
		if( m_NodeCurrent.eFactID != kFactID_Invalid )
		{
			// Clear ALL MountVehicle tasks.

			m_pAI->GetAIBlackBoard()->SetBBTaskStatus( kTaskStatus_Done );
			CAIWMFact factQuery;
			factQuery.SetFactType( kFact_Task );
			factQuery.SetTaskType( kTask_MountVehicle );
			m_pAI->GetAIWorkingMemory()->ClearWMFacts( factQuery );
			m_NodeCurrent.eFactID = kFactID_Invalid;
		}
	}
}
