// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalInvestigate.cpp
//
// PURPOSE : AIGoalInvestigate class implementation
//
// CREATED : 3/25/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIGoalInvestigate.h"
#include "AI.h"
#include "AIDB.h"
#include "AIWorkingMemory.h"
#include "AIBlackBoard.h"
#include "AIStimulusMgr.h"
#include "AITarget.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Goal, CAIGoalInvestigate, kGoal_Investigate );


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalInvestigate::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAIGoalInvestigate::CAIGoalInvestigate()
{
	m_eDisturbanceID = kStimID_Unset;
	m_hTargetObject = NULL;
}

CAIGoalInvestigate::~CAIGoalInvestigate()
{
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIGoalInvestigate::Save/Load
//              
//	PURPOSE:	Handle saving and restoring the CAIGoalInvestigate
//              
//----------------------------------------------------------------------------

void CAIGoalInvestigate::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);

	SAVE_INT(m_eDisturbanceID);
	SAVE_HOBJECT(m_hTargetObject);
}

void CAIGoalInvestigate::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);

	LOAD_INT_CAST(m_eDisturbanceID, EnumAIStimulusID);
	LOAD_HOBJECT(m_hTargetObject);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalInvestigate::CalculateGoalRelevance
//
//	PURPOSE:	Calculate the current goal relevance.
//
// ----------------------------------------------------------------------- //

void CAIGoalInvestigate::CalculateGoalRelevance()
{
	// AI is aware of a disturbance.

	if( m_pAI->HasTarget( kTarget_Disturbance ) )
	{
		m_fGoalRelevance = m_pGoalRecord->fIntrinsicRelevance;
		return;
	}

	// No disturbance present.

	m_fGoalRelevance = 0.f;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalInvestigate::ReplanRequired
//
//	PURPOSE:	Return true if AI needs to make a new plan.
//
// ----------------------------------------------------------------------- //

bool CAIGoalInvestigate::ReplanRequired()
{
	// Replan if the target has changed to a new stimulus.

	if(	m_eDisturbanceID != m_pAI->GetAIBlackBoard()->GetBBTargetStimulusID() )
	{
		return true;
	}

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalInvestigate::ActivateGoal
//
//	PURPOSE:	Activate the goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalInvestigate::ActivateGoal()
{
	super::ActivateGoal();

	AITRACE( AIShowGoals, ( m_pAI->m_hObject, "Investigating stimulus: %s",
		g_pAIStimulusMgr->StimulusToString( m_pAI->GetAIBlackBoard()->GetBBTargetStimulusType() ) ) );

	// Record which stimulus AI is responding to.

	m_eDisturbanceID = m_pAI->GetAIBlackBoard()->GetBBTargetStimulusID();
	m_hTargetObject = m_pAI->GetAIBlackBoard()->GetBBTargetObject();

	// Reserve links along path to the disturbance.

	m_pAI->GetAIBlackBoard()->SetBBReserveNMLinks( true );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalInvestigate::DeactivateGoal
//
//	PURPOSE:	Deactivate the goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalInvestigate::DeactivateGoal()
{
	super::DeactivateGoal();

	if ( IsWSSatisfied( m_pAI->GetAIWorldState() ) )
	{
		CAIWMFact factQuery;
		factQuery.SetFactType( kFact_Disturbance );
		factQuery.SetTargetObject( m_hTargetObject );
		m_pAI->GetAIWorkingMemory()->ClearWMFact( factQuery );
	}

	// Always reevaluate the target, even if the goal isn't satisfied.  
	// This insures we don't accidently end up with a disturbance without a 
	// fact as the target.

	m_pAI->GetAIBlackBoard()->SetBBSelectTarget( true );

	m_eDisturbanceID = kStimID_Unset;
	m_hTargetObject = NULL;

	// Remove reservations from links along path to the disturbance.

	m_pAI->GetAIBlackBoard()->SetBBReserveNMLinks( false );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalInvestigate::HandleBuildPlanFailure
//
//	PURPOSE:	Handle plan construction failing to find a valid plan to
//				accomplish this goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalInvestigate::HandleBuildPlanFailure()
{
	super::HandleBuildPlanFailure();

	// Do not repeatedly try to react to a disturbance if we failed.

	CAIWMFact factQuery;
	factQuery.SetFactType( kFact_Disturbance );
	factQuery.SetTargetObject( m_hTargetObject );
	m_pAI->GetAIWorkingMemory()->ClearWMFact( factQuery );

	m_eDisturbanceID = kStimID_Unset;
	m_hTargetObject = NULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalInvestigate::SetWSSatisfaction
//
//	PURPOSE:	Set the WorldState satisfaction conditions.
//
// ----------------------------------------------------------------------- //

void CAIGoalInvestigate::SetWSSatisfaction( CAIWorldState& WorldState )
{
	WorldState.SetWSProp( kWSK_DisturbanceExists, m_pAI->m_hObject, kWST_bool, false );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalInvestigate::IsWSSatisfied
//
//	PURPOSE:	Return true if the world state satisfies the goal.
//
// ----------------------------------------------------------------------- //

bool CAIGoalInvestigate::IsWSSatisfied( CAIWorldState* pwsWorldState )
{
	SAIWORLDSTATE_PROP* pProp = pwsWorldState->GetWSProp( kWSK_DisturbanceExists, m_pAI->m_hObject );
	if( pProp && ( !pProp->bWSValue ) )
	{
		return true;
	}

	return false;
}

