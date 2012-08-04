// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalChaseBerserker.cpp
//
// PURPOSE : This goal handles the desire to run at the enemy in 
//			 preparation for a berserker attack.  This goal is lower
//			 priority than the actual berserker attack itself.
//
// CREATED : 8/10/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIGoalChaseBerserker.h"
#include "AI.h"
#include "AIBlackBoard.h"

LINKFROM_MODULE(AIGoalChaseBerserker);

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Goal, CAIGoalChaseBerserker, kGoal_ChaseBerserker );

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalChaseBerserker::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAIGoalChaseBerserker::CAIGoalChaseBerserker()
{
}

CAIGoalChaseBerserker::~CAIGoalChaseBerserker()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalChaseBerserker::CalculateGoalRelevance
//
//	PURPOSE:	Calculate the current goal relevance.
//
// ----------------------------------------------------------------------- //

void CAIGoalChaseBerserker::CalculateGoalRelevance()
{
	// Do not chase if AI is attached to something.

	if( m_pAI->GetAIBlackBoard()->GetBBAttachedTo() != NULL )
	{
		m_fGoalRelevance = 0.f;
		return;
	}

	// Do not chase if the AI does not have a character target.

	if( !m_pAI->HasTarget( kTarget_Berserker ) )
	{
		m_fGoalRelevance = 0.f;
		return;
	}

	// AI has a weapon

	if ( AIWeaponUtils::HasWeaponType( m_pAI, kAIWeaponType_Melee, AIWEAP_CHECK_HOLSTER ) 
		|| AIWeaponUtils::HasWeaponType( m_pAI, kAIWeaponType_Ranged, AIWEAP_CHECK_HOLSTER ) )
	{
		m_fGoalRelevance = 0.f;
		return;
	}

	// ChaseBerserker!

	m_fGoalRelevance = m_pGoalRecord->fIntrinsicRelevance;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalChase::HandleBuildPlanFailure
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

void CAIGoalChaseBerserker::HandleBuildPlanFailure()
{
	super::HandleBuildPlanFailure();

	// Time out the desire to perform a berserker attack against this 
	// enemy.  This may happen if the AI cannot path to the enemy.
	// TODO: If this value ever needs to change, make it data driven.

	CAIWMFact factQuery;
	factQuery.SetFactType( kFact_Desire );
	factQuery.SetDesireType( kDesire_Berserker );
	factQuery.SetTargetObject( m_pAI->GetAIBlackBoard()->GetBBTargetObject() );
	CAIWMFact* pBerserkerDesire = m_pAI->GetAIWorkingMemory()->FindWMFact( factQuery );
	if ( pBerserkerDesire )
	{
		pBerserkerDesire->SetTime( g_pLTServer->GetTime() + 2.0f );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalChase::SetWSSatisfaction
//
//	PURPOSE:	Set the WorldState satisfaction conditions.
//
// ----------------------------------------------------------------------- //

void CAIGoalChaseBerserker::SetWSSatisfaction( CAIWorldState& WorldState )
{
	WorldState.SetWSProp( kWSK_AtTargetPos, m_pAI->m_hObject, kWST_bool, true );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalChaseBerserker::IsWSSatisfied
//
//	PURPOSE:	Return true if the world state satisfies the goal.
//
// ----------------------------------------------------------------------- //

bool CAIGoalChaseBerserker::IsWSSatisfied( CAIWorldState* pwsWorldState )
{
	SAIWORLDSTATE_PROP* pProp = NULL;

	pProp = pwsWorldState->GetWSProp( kWSK_AtTargetPos, m_pAI->m_hObject );
	if( !pProp || ( pProp && !pProp->bWSValue ) )
	{
		return false;
	}

	return true;
}
