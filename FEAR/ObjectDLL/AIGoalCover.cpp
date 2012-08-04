// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalCover.cpp
//
// PURPOSE : AIGoalCover class implementation
//
// CREATED : 3/13/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIGoalCover.h"
#include "AI.h"
#include "AIBlackBoard.h"
#include "AICoordinator.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Goal, CAIGoalCover, kGoal_Cover );


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalCover::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAIGoalCover::CAIGoalCover()
{
	m_eTaskType = kTask_Cover;
	m_bClearScriptedTaskIfThreatened = false;
}

CAIGoalCover::~CAIGoalCover()
{
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIGoalCover::Save/Load
//              
//	PURPOSE:	Handle saving and restoring the CAIGoalGoto
//              
//----------------------------------------------------------------------------

void CAIGoalCover::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);
}

void CAIGoalCover::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalCover::CalculateGoalRelevance
//
//	PURPOSE:	Calculate the current goal relevance.
//
// ----------------------------------------------------------------------- //

void CAIGoalCover::CalculateGoalRelevance()
{
	// Do not take cover if AI is attached to something.

	if( m_pAI->GetAIBlackBoard()->GetBBAttachedTo() != NULL )
	{
		m_fGoalRelevance = 0.f;
		return;
	}

	// Do not take cover if AI has the desire to hold position.

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
//	ROUTINE:	CAIGoalCover::ActivateGoal
//
//	PURPOSE:	Activate the goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalCover::ActivateGoal()
{
	super::ActivateGoal();

	// Try to take the safest path to the cover node.

	m_pAI->GetAIBlackBoard()->SetBBPathType( kPath_Safe );

	// Reserve links along path to the cover node.

	m_pAI->GetAIBlackBoard()->SetBBReserveNMLinks( true );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalCover::DeactivateGoal
//
//	PURPOSE:	Deactivate the goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalCover::DeactivateGoal()
{
	super::DeactivateGoal();

	// Default pathfinding.

	m_pAI->GetAIBlackBoard()->SetBBPathType( kPath_Default );

	// Remove reservations from links along path to the cover node.

	m_pAI->GetAIBlackBoard()->SetBBReserveNMLinks( false );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalCover::SetWSSatisfaction
//
//	PURPOSE:	Set the WorldState satisfaction conditions.
//
// ----------------------------------------------------------------------- //

void CAIGoalCover::SetWSSatisfaction( CAIWorldState& WorldState )
{
	// Intentionally do not call super::SetWSSatisfaction.
	// Satisfaction requires UsingNode rather than AtNode.
	// This forces the AI to handle node dependencies.

	// Set satisfaction properties.

	WorldState.SetWSProp( kWSK_UsingObject, m_pAI->m_hObject, kWST_HOBJECT, m_NodePending.hNode );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalCover::UpdateTaskStatus
//
//	PURPOSE:	Update the current status of the task.
//
// ----------------------------------------------------------------------- //

void CAIGoalCover::UpdateTaskStatus()
{
	// Do not consider Task to be scripted once the AI 
	// takes damage or actually sees his target.
	// Bail if squad enters combat.

	if( m_NodeCurrent.bTaskIsScripted )
	{
		// Bail on scripted cover if threat is visible.

		bool bKillScript = m_pAI->GetAIBlackBoard()->GetBBTargetVisibleFromEye();

		// Bail on scripted cover if AI has taken damage.

		CAIWMFact factQuery;
		factQuery.SetFactType(kFact_Damage);
		if( m_pAI->GetAIWorkingMemory()->FindWMFact( factQuery ) )
		{
			bKillScript = true;
		}

		// Bail on scripted cover if squad has engaged in combat.

		ENUM_AI_SQUAD_ID eSquadID = g_pAICoordinator->GetSquadID( m_pAI->m_hObject );
		CAISquad* pSquad = g_pAICoordinator->FindSquad( eSquadID );
		if( pSquad &&
			( pSquad->IsSquadMemberDead() || 
			  pSquad->HasTarget( kTarget_Character | kTarget_Object ) ) )
		{
			bKillScript = true;
		}

		// Something significant has happened.  Set the AI free.

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
