// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalAmbush.cpp
//
// PURPOSE : AIGoalAmbush class implementation
//
// CREATED : 3/01/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIGoalAmbush.h"
#include "AI.h"
#include "AINode.h"
#include "AIBlackBoard.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Goal, CAIGoalAmbush, kGoal_Ambush );


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalAmbush::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAIGoalAmbush::CAIGoalAmbush()
{
	m_eTaskType = kTask_Ambush;
	m_bClearScriptedTaskIfThreatened = false;
	m_bThreatSighted = false;
}

CAIGoalAmbush::~CAIGoalAmbush()
{
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIGoalAmbush::Save/Load
//              
//	PURPOSE:	Handle saving and restoring the CAIGoalGoto
//              
//----------------------------------------------------------------------------

void CAIGoalAmbush::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);

	SAVE_bool( m_bThreatSighted );
}

void CAIGoalAmbush::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);

	LOAD_bool( m_bThreatSighted );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalAmbush::CalculateGoalRelevance
//
//	PURPOSE:	Calculate the current goal relevance.
//
// ----------------------------------------------------------------------- //

void CAIGoalAmbush::CalculateGoalRelevance()
{
	// Do not ambush if AI is attached to something.

	if( m_pAI->GetAIBlackBoard()->GetBBAttachedTo() != NULL )
	{
		m_fGoalRelevance = 0.f;
		return;
	}

	// Do not ambush if AI has the desire to hold position.

	CAIWMFact factQuery;
	factQuery.SetFactType( kFact_Desire );
	factQuery.SetDesireType( kDesire_HoldPosition );
	if( m_pAI->GetAIWorkingMemory()->FindWMFact( factQuery ) )
	{
		m_fGoalRelevance = 0.f;
		return;
	}

	// Default calculation.

	super::CalculateGoalRelevance();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalAmbush::SetWSSatisfaction
//
//	PURPOSE:	Set the WorldState satisfaction conditions.
//
// ----------------------------------------------------------------------- //

void CAIGoalAmbush::SetWSSatisfaction( CAIWorldState& WorldState )
{
	// Intentionally do not call super::SetWSSatisfaction.
	// Satisfaction requires UsingNode rather than AtNode.
	// This forces the AI to handle node dependencies.

	// Set satisfaction properties.

	WorldState.SetWSProp( kWSK_UsingObject, m_pAI->m_hObject, kWST_HOBJECT, m_NodePending.hNode );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalAmbush::ActivateGoal
//
//	PURPOSE:	Activate the goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalAmbush::ActivateGoal()
{
	super::ActivateGoal();

	m_bThreatSighted = false;

	// Try to take the safest path to the ambush node.

	m_pAI->GetAIBlackBoard()->SetBBPathType( kPath_Safe );

	// Reserve links along path to the ambush node.

	m_pAI->GetAIBlackBoard()->SetBBReserveNMLinks( true );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalAmbush::DeactivateGoal
//
//	PURPOSE:	Deactivate the goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalAmbush::DeactivateGoal()
{
	super::DeactivateGoal();

	// Default pathfinding.

	m_pAI->GetAIBlackBoard()->SetBBPathType( kPath_Default );

	// Remove reservations from links along path to the ambush node.

	m_pAI->GetAIBlackBoard()->SetBBReserveNMLinks( false );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalAmbush::UpdateTaskStatus
//
//	PURPOSE:	Update the current status of the task.
//
// ----------------------------------------------------------------------- //

void CAIGoalAmbush::UpdateTaskStatus()
{
	// Do not consider Task to be scripted once the AI has actually
	// seen his target, and the ambush node is invalid.

	if( m_NodeCurrent.bTaskIsScripted )
	{

		// Bail on scripted cover if AI has taken damage.

		bool bKillScript = false;
		CAIWMFact factQuery;
		factQuery.SetFactType(kFact_Damage);
		if( m_pAI->GetAIWorkingMemory()->FindWMFact( factQuery ) )
		{
			bKillScript = true;
		}

		// Target spotted.

		if( m_pAI->GetAIBlackBoard()->GetBBTargetVisibleFromEye() )
		{
			m_bThreatSighted = true;
		}
		
		// No longer consider the goal to be scripted once the 
		// AI has seen the target and the node has been invalidated.

		if( m_bThreatSighted )
		{
			// Do not consider the node scripted if the target 
			// goes out of sight.

			if( !m_pAI->GetAIBlackBoard()->GetBBTargetVisibleFromEye() )
			{
				m_NodeCurrent.bTaskIsScripted = false;
			}

			// Do not consider the node scripted if the target 
			// invalidates the node.

			else {
				HOBJECT hTarget = m_pAI->GetAIBlackBoard()->GetBBTargetObject();

				AINode* pNode = (AINode*)g_pLTServer->HandleToObject( m_NodeCurrent.hNode );
				if( pNode && !pNode->IsNodeValid( m_pAI, m_pAI->GetPosition(), hTarget, kThreatPos_TargetPos, kNodeStatus_All & ~( kNodeStatus_Expired | kNodeStatus_ThreatOutsideFOV ) ) )
				{
					bKillScript = true;
				}
			}
		}

		// Set the AI free due to current situation.

		if( bKillScript )
		{
			CAIWMFact factQuery;
			factQuery.SetFactType( kFact_Task );
			factQuery.SetTaskType( m_eTaskType );
			CAIWMFact* pFact = m_pAI->GetAIWorkingMemory()->FindWMFact( factQuery );
			if( pFact )
			{
				uint32 dwFactFlags = pFact->GetFactFlags();
				pFact->SetFactFlags( dwFactFlags & ~kFactFlag_Scripted );
			}

			m_NodeCurrent.bTaskIsScripted = false;
		}
	}
	
	// Default behavior.

	super::UpdateTaskStatus();
}
