// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalDeath.cpp
//
// PURPOSE : AIGoalDeath class implementation
//
// CREATED : 04/15/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIGoalDeath.h"
#include "AI.h"
#include "AIDB.h"
#include "AIUtils.h"


DEFINE_AI_FACTORY_CLASS_SPECIFIC( Goal, CAIGoalDeath, kGoal_Death );


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalDeath::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAIGoalDeath::CAIGoalDeath()
{
}

CAIGoalDeath::~CAIGoalDeath()
{
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIGoalDeath::Save/Load
//              
//	PURPOSE:	Handle saving and restoring the Goal
//              
//----------------------------------------------------------------------------

void CAIGoalDeath::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);
}

void CAIGoalDeath::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalDeath::CalculateGoalRelevance
//
//	PURPOSE:	Calculate the current goal relevance.
//
// ----------------------------------------------------------------------- //

void CAIGoalDeath::CalculateGoalRelevance()
{
	// Goal is relevant if we are dead!

	if( m_pAI->GetDestructible()->IsDead() )
	{
		m_fGoalRelevance = m_pGoalRecord->fIntrinsicRelevance;
		return;
	}

	// We are not dead.

	m_fGoalRelevance = 0.f;
}
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalDeath::HandleBuildPlanFailure
//
//	PURPOSE:	Handle plan construction failing to find a valid plan to
//				accomplish this goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalDeath::HandleBuildPlanFailure()
{
	super::HandleBuildPlanFailure();

	StartDeath();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalDeath::ActivateGoal
//
//	PURPOSE:	Activate the goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalDeath::ActivateGoal()
{
	super::ActivateGoal();

	StartDeath();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalDeath::DeactivateGoal
//
//	PURPOSE:	Deactivate the goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalDeath::DeactivateGoal()
{
	super::DeactivateGoal();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalDeath::StartDeath
//
//	PURPOSE:	Setup the AI for death.
//
// ----------------------------------------------------------------------- //

void CAIGoalDeath::StartDeath()
{
	AITRACE( AIShowStates, ( m_pAI->m_hObject, "AI is Dead" ) );

	if( m_pAI->IsControlledByDialogue() )
	{
		m_pAI->StopDialogue();
	}

	if( m_pAI->WasSilentKill() )
	{
		m_pAI->SetLastPainVolume( 0.1f );
	}

	// Move to wherever our torso is

	m_pAI->SyncPosition();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalDeath::UpdateGoal
//
//	PURPOSE:	Return true if goal should keep running.
//
// ----------------------------------------------------------------------- //

bool CAIGoalDeath::UpdateGoal()
{
	super::UpdateGoal();

	// Always keep running.
	// Death never stops!

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalDeath::UpdateGoal
//
//	PURPOSE:	Return true if plan is still valid to continue running.
//
// ----------------------------------------------------------------------- //

bool CAIGoalDeath::IsPlanValid()
{
	// Always keep running.
	// Death never stops!

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalDeath::SetWSSatisfaction
//
//	PURPOSE:	Set the WorldState satisfaction conditions.
//
// ----------------------------------------------------------------------- //

void CAIGoalDeath::SetWSSatisfaction( CAIWorldState& WorldState )
{
	WorldState.SetWSProp( kWSK_ReactedToWorldStateEvent, m_pAI->m_hObject, kWST_ENUM_AIWorldStateEvent, kWSE_Damage );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalDeath::IsWSSatisfied
//
//	PURPOSE:	Return true if the world state satisfies the goal.
//
// ----------------------------------------------------------------------- //

bool CAIGoalDeath::IsWSSatisfied( CAIWorldState* pwsWorldState )
{
	// We have handled the damage.

	SAIWORLDSTATE_PROP* pProp = pwsWorldState->GetWSProp( kWSK_ReactedToWorldStateEvent, m_pAI->m_hObject );
	if( pProp && ( pProp->eAIWorldStateEventWSValue == kWSE_Damage  ) )
	{
		return true;
	}

	// We have not handled the damage.

	return false;
}


