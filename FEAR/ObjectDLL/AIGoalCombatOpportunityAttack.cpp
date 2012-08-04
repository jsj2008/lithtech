// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalCombatOpportunityAttack.cpp
//
// PURPOSE : This goal handles use of an AICombatOpportunity as the AIs 
//			 target.  This allows an AI to formulate a plan to kill the
//			 AICombatOpporunities TargetObject.  If the AI fails to build
//			 a plan to do this, the AICombatOpportunity specifying this 
//			 target is found, its next evaluation time set forward.
//
// CREATED : 6/11/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIGoalCombatOpportunityAttack.h"
#include "AI.h"
#include "AITarget.h"
#include "AIDB.h"
#include "AIWorldState.h"
#include "AIWorkingMemory.h"
#include "AIBlackBoard.h"

LINKFROM_MODULE(AIGoalCombatOpportunityAttack);


DEFINE_AI_FACTORY_CLASS_SPECIFIC( Goal, CAIGoalCombatOpportunityAttack, kGoal_CombatOpportunity );

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalCombatOpportunityAttack::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAIGoalCombatOpportunityAttack::CAIGoalCombatOpportunityAttack()
{
}

CAIGoalCombatOpportunityAttack::~CAIGoalCombatOpportunityAttack()
{
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIGoalCombatOpportunityAttack::Save/Load
//              
//	PURPOSE:	Handle saving and restoring the CAIGoalCombatOpportunityAttack
//              
//----------------------------------------------------------------------------

void CAIGoalCombatOpportunityAttack::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);
}

void CAIGoalCombatOpportunityAttack::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIGoalCombatOpportunityAttack::Save/Load
//              
//	PURPOSE:	Handle saving and restoring the CAIGoalCombatOpportunityAttack
//              
//----------------------------------------------------------------------------

void CAIGoalCombatOpportunityAttack::CalculateGoalRelevance()
{
	// Current target is not a combat opportunity.

	if ( !(kTarget_CombatOpportunity & m_pAI->GetAIBlackBoard()->GetBBTargetType()) )
	{
		m_fGoalRelevance = 0.f;	
		return;
	}

	// Goal is relevant.

	m_fGoalRelevance = m_pGoalRecord->fIntrinsicRelevance;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalCombatOpportunityAttack::SetWSSatisfaction
//
//	PURPOSE:	Set the WorldState satisfaction conditions.
//
// ----------------------------------------------------------------------- //

void CAIGoalCombatOpportunityAttack::HandleBuildPlanFailure()
{
	super::HandleBuildPlanFailure();

	// Failed to find a plan to attack the combat opportunity.  Reset the
	// combat opportunities timer.
	//
	// NOTE: The below assumes that a single combat opportunity 'owns' a target.
	// If multiple combat opportunities point at the same target, this may need
	// to be revised.

	CAIWMFact queryFact;
	queryFact.SetFactType(kFact_Knowledge);
	queryFact.SetKnowledgeType(kKnowledge_CombatOpportunity);
	queryFact.SetTargetObject(m_pAI->GetAIBlackBoard()->GetBBTargetObject());
	CAIWMFact* pFact = m_pAI->GetAIWorkingMemory()->FindWMFact(queryFact);
	if (pFact)
	{
		// TODO: If this needs to get modified, move it to the the Const 
		// record in the database, along with any other SetTime modifications
		// of this fact.
		pFact->SetTime(g_pLTServer->GetTime() + 2.0f);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalCombatOpportunityAttack::ActivateGoal
//
//	PURPOSE:	Activate the goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalCombatOpportunityAttack::ActivateGoal()
{
	super::ActivateGoal();

	// Don't intentionally miss while firing at a combat opportunity.

	m_pAI->GetAIBlackBoard()->SetBBPerfectAccuracy( true );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalCombatOpportunityAttack::DeactivateGoal
//
//	PURPOSE:	Deactivate the goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalCombatOpportunityAttack::DeactivateGoal()
{
	super::DeactivateGoal();

	// Default behavior.

	m_pAI->GetAIBlackBoard()->SetBBPerfectAccuracy( false );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalCombatOpportunityAttack::SetWSSatisfaction
//
//	PURPOSE:	Set the WorldState satisfaction conditions.
//
// ----------------------------------------------------------------------- //

void CAIGoalCombatOpportunityAttack::SetWSSatisfaction( CAIWorldState& WorldState )
{
	WorldState.SetWSProp( kWSK_TargetIsDead, m_pAI->m_hObject, kWST_bool, true );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalCombatOpportunityAttack::IsWSSatisfied
//
//	PURPOSE:	Return true if the world state satisfies the goal.
//
// ----------------------------------------------------------------------- //

bool CAIGoalCombatOpportunityAttack::IsWSSatisfied( CAIWorldState* pwsWorldState )
{
	SAIWORLDSTATE_PROP* pProp = pwsWorldState->GetWSProp( kWSK_TargetIsDead, m_pAI->m_hObject );
	if( pProp && ( pProp->bWSValue ) )
	{
		return true;
	}

	return false;
}
