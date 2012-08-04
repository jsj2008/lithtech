// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalGoto.cpp
//
// PURPOSE : AIGoalGoto class implementation
//
// CREATED : 3/03/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIGoalGoto.h"
#include "AI.h"
#include "AIDB.h"
#include "AINode.h"
#include "AIBlackBoard.h"
#include "AIWorkingMemory.h"
#include "AIUtils.h"


DEFINE_AI_FACTORY_CLASS_SPECIFIC( Goal, CAIGoalGoto, kGoal_Goto );


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SAI_GOTO_REQUEST::Constructor
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

SAI_GOTO_REQUEST::SAI_GOTO_REQUEST()
{
	eFactID = kFactID_Invalid;
	hNode = NULL;
	bTaskIsScripted = false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalGoto::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAIGoalGoto::CAIGoalGoto()
{
	m_eTaskType = kTask_Goto;
	m_bCheckNodeValidity = false;
	m_bClearScriptedTaskIfThreatened = true;
}

CAIGoalGoto::~CAIGoalGoto()
{
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIGoalGoto::Save/Load
//              
//	PURPOSE:	Handle saving and restoring the CAIGoalGoto
//              
//----------------------------------------------------------------------------

void CAIGoalGoto::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);

	SAVE_DWORD( m_NodeCurrent.eFactID );
	SAVE_HOBJECT( m_NodeCurrent.hNode );
	SAVE_bool( m_NodeCurrent.bTaskIsScripted );

	SAVE_DWORD( m_NodePending.eFactID );
	SAVE_HOBJECT( m_NodePending.hNode );
	SAVE_bool( m_NodePending.bTaskIsScripted );
}

void CAIGoalGoto::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);

	LOAD_DWORD_CAST( m_NodeCurrent.eFactID, ENUM_FactID );
	LOAD_HOBJECT( m_NodeCurrent.hNode );
	LOAD_bool( m_NodeCurrent.bTaskIsScripted );

	LOAD_DWORD_CAST( m_NodePending.eFactID, ENUM_FactID );
	LOAD_HOBJECT( m_NodePending.hNode );
	LOAD_bool( m_NodePending.bTaskIsScripted );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalGoto::CalculateGoalRelevance
//
//	PURPOSE:	Calculate the current goal relevance.
//
// ----------------------------------------------------------------------- //

void CAIGoalGoto::CalculateGoalRelevance()
{
	// Wait until after teleporting to Goto.
	// This is necessary due to update ordering between CAI and CCharacter.

	if( m_pAI->GetTeleportTriggerState() != eTeleporTriggerStateNone )
	{
		m_fGoalRelevance = 0.f;
		return;
	}

	// Bail if we have no Goto task.

	CAIWMFact factQuery;
	factQuery.SetFactType( kFact_Task );
	factQuery.SetTaskType( m_eTaskType );
	CAIWMFact* pFact = m_pAI->GetAIWorkingMemory()->FindWMFact( factQuery );
	if( pFact && 
		pFact->IsSet( CAIWMFact::kFactMask_TargetObject ) &&
		pFact->GetTargetObject() &&
		( pFact->GetConfidence( CAIWMFact::kFactMask_TaskType ) == 1.f ) )
	{
		// Bail if node is disabled or invalid.

		HOBJECT hNode = pFact->GetTargetObject();
		if( IsAINode( hNode ) )
		{
			// Node is disabled.

			AINode* pNode = (AINode*)g_pLTServer->HandleToObject( hNode );
			if( pNode->IsNodeDisabled() )
			{
				hNode = NULL;
				m_fGoalRelevance = 0.f;
				return;
			}

			// Node is invalid.

			if( m_bCheckNodeValidity &&
				!pNode->IsNodeValid( m_pAI, m_pAI->GetPosition(), NULL, kThreatPos_TargetPos, kNodeStatus_All ) )
			{
				hNode = NULL;
				m_fGoalRelevance = 0.f;
				return;
			}
		}

		// New node requested.

		if( m_NodeCurrent.eFactID != pFact->GetFactID() )
		{
			m_NodePending.hNode = hNode;
			m_NodePending.eFactID = pFact->GetFactID();
			m_NodePending.bTaskIsScripted = ( pFact->GetFactFlags() & kFactFlag_Scripted ) ? true : false;
		}
		m_fGoalRelevance = m_pGoalRecord->fIntrinsicRelevance;
		return;
	}

	// There is no Goto task.

	m_fGoalRelevance = 0.f;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalGoto::ReplanRequired
//
//	PURPOSE:	Return true if AI needs to make a new plan.
//
// ----------------------------------------------------------------------- //

bool CAIGoalGoto::ReplanRequired()
{
	// Replan if the destination has changed.

	if(	m_NodePending.hNode != m_NodeCurrent.hNode )
	{
		return true;
	}

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalGoto::ActivateGoal
//
//	PURPOSE:	Activate the goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalGoto::ActivateGoal()
{
	super::ActivateGoal();

	m_NodeCurrent = m_NodePending;

	m_pAI->GetAIBlackBoard()->SetBBTaskStatus( kTaskStatus_Set );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalGoto::DeactivateGoal
//
//	PURPOSE:	Deactivate the goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalGoto::DeactivateGoal()
{
	super::DeactivateGoal();

	// Bail on a scripted Goto if we're targeting a threat, and have seen the threat.

	if( m_bClearScriptedTaskIfThreatened &&
		m_NodeCurrent.bTaskIsScripted &&
		m_pAI->HasTarget( kTarget_Character | kTarget_Object ) &&
		( m_pAI->GetAIBlackBoard()->GetBBTargetLastVisibleTime() > 0.f ) &&
		( m_pAI->GetAIBlackBoard()->GetBBSensesOn() ) )
	{
		ClearGotoTask( &m_NodeCurrent );
	}


	m_NodeCurrent.eFactID = kFactID_Invalid;
	m_NodeCurrent.hNode = NULL;
	m_NodeCurrent.bTaskIsScripted = false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalGoto::SetWSSatisfaction
//
//	PURPOSE:	Set the WorldState satisfaction conditions.
//
// ----------------------------------------------------------------------- //

void CAIGoalGoto::SetWSSatisfaction( CAIWorldState& WorldState )
{
	// Set satisfaction properties.

	WorldState.SetWSProp( kWSK_AtNode, m_pAI->m_hObject, kWST_HOBJECT, m_NodePending.hNode );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalGoto::IsWSSatisfied
//
//	PURPOSE:	Return true if the world state satisfies the goal.
//
// ----------------------------------------------------------------------- //

bool CAIGoalGoto::IsWSSatisfied( CAIWorldState* pwsWorldState )
{
	// Goal is never satisfied if scripted Task exists.

	if( m_NodeCurrent.bTaskIsScripted )
	{
		return false;
	}

	// Goal is not satisfied yet if AI still needs to mount the node.

	if( IsAINodeSmartObject( m_NodeCurrent.hNode ) )
	{
		AINodeSmartObject* pNodeSmartObject = (AINodeSmartObject*)g_pLTServer->HandleToObject( m_NodeCurrent.hNode );
		if( pNodeSmartObject && pNodeSmartObject->GetSmartObject() )
		{
			AIDB_SmartObjectRecord* pSmartObject = pNodeSmartObject->GetSmartObject();
			if( pSmartObject->Props.Get( kAPG_Posture ) == kAP_POS_Mounted )
			{
				SAIWORLDSTATE_PROP* pMountedProp = m_pAI->GetAIWorldState()->GetWSProp( kWSK_MountedObject );
				if( !( pMountedProp && pMountedProp->hWSValue == m_NodeCurrent.hNode ) )
				{
					return false;
				}
			}
		}
	}

	// Goal is satisfied if we are at the node.

	SAIWORLDSTATE_PROP* pProp = pwsWorldState->GetWSProp( kWSK_AtNode, m_pAI->m_hObject );
	if( pProp && pProp->hWSValue && ( pProp->hWSValue == m_NodeCurrent.hNode ) )
	{
		return true;
	}

	// Not at the node.

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalGoto::UpdateTaskStatus
//
//	PURPOSE:	Update the current status of the task.
//
// ----------------------------------------------------------------------- //

void CAIGoalGoto::UpdateTaskStatus()
{
	// Clear completed task.

	if( IsWSSatisfied( m_pAI->GetAIWorldState() ) )
	{
		ClearGotoTask( &m_NodeCurrent );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalGoto::HandleBuildPlanFailure
//
//	PURPOSE:	Handle plan construction failing to find a valid plan to
//				accomplish this goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalGoto::HandleBuildPlanFailure()
{
	super::HandleBuildPlanFailure();

	ClearGotoTask( &m_NodePending );
	m_NodePending.eFactID = kFactID_Invalid;
	m_NodePending.hNode = NULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalGoto::ClearGotoTask
//
//	PURPOSE:	Clear the existing task that made this goal relevant.
//
// ----------------------------------------------------------------------- //

void CAIGoalGoto::ClearGotoTask( SAI_GOTO_REQUEST* pGotoRequest )
{
	// Sanity check.

	if( !pGotoRequest )
	{
		return;
	}

	// Clear the task.

	if( pGotoRequest->eFactID != kFactID_Invalid )
	{
		m_pAI->GetAIBlackBoard()->SetBBTaskStatus( kTaskStatus_Done );

		CAIWMFact factQuery;
		factQuery.SetFactID( pGotoRequest->eFactID );
		m_pAI->GetAIWorkingMemory()->ClearWMFact( factQuery );
	}
}

