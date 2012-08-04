// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalDodge.cpp
//
// PURPOSE : AIGoalDodge class implementation
//
// CREATED : 3/14/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIGoalDodge.h"
#include "AI.h"
#include "AIDB.h"
#include "AIBlackBoard.h"
#include "AIGoalMgr.h"
#include "AIWorkingMemory.h"
#include "AIUtils.h"
#include "AnimationContext.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Goal, CAIGoalDodge, kGoal_Dodge );


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalDodge::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAIGoalDodge::CAIGoalDodge()
{
	m_bAlwaysDodgeWhenShot = true;
}

CAIGoalDodge::~CAIGoalDodge()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalDodge::SetNextRecalcTime
//
//	PURPOSE:	Set the next time to recalculate the current goal relevance.
//
// ----------------------------------------------------------------------- //

void CAIGoalDodge::SetNextRecalcTime()
{
	// Override the default SetNextRecalcTime to take into account the difficulty factor.

	m_fNextRecalcTime = g_pLTServer->GetTime() + 
						GetRandom( LOWER_BY_DIFFICULTY( m_pGoalRecord->fRecalcRateMin ), 
								   LOWER_BY_DIFFICULTY( m_pGoalRecord->fRecalcRateMax ) );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalDodge::GetNextRecalcTime
//
//	PURPOSE:	Return the next time to recalculate the current goal relevance.
//
// ----------------------------------------------------------------------- //

double CAIGoalDodge::GetNextRecalcTime()
{
	// HACK: This is a complete hack for DarkAlma.

	if( m_pAI->GetAIBlackBoard()->GetBBHandleTouch() == kTouch_Damage )
	{
		m_bAlwaysDodgeWhenShot = false;
	}

	// END HACK

	// Recalculate imediately if we were just shot.

	if( m_bAlwaysDodgeWhenShot )
	{
		CAIWMFact factQuery;
		factQuery.SetFactType(kFact_Damage);
		CAIWMFact* pFact = m_pAI->GetAIWorkingMemory()->FindWMFact( factQuery );
		if( pFact && DidDamage(m_pAI, pFact) )
		{
			double fCurTime = g_pLTServer->GetTime();
			if( fCurTime - pFact->GetUpdateTime() < 1.f )
			{
				return 0.f;
			}
		}
	}

	// Default behavior.

	return super::GetNextRecalcTime();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalDodge::GetActivateChance
//
//	PURPOSE:	Return the probability of this goal activating.
//
// ----------------------------------------------------------------------- //

float CAIGoalDodge::GetActivateChance() const
{
	// Override the default GetActivateChance to take into account the difficulty factor.

	return RAISE_BY_DIFFICULTY( m_pGoalRecord->fActivateChance );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalDodge::CalculateGoalRelevance
//
//	PURPOSE:	Calculate the current goal relevance.
//
// ----------------------------------------------------------------------- //

void CAIGoalDodge::CalculateGoalRelevance()
{
	// Already dodging.

	if( m_pAI->GetGoalMgr()->IsCurGoal( this ) )
	{
		if (!IsWSSatisfied(m_pAI->GetAIWorldState()))
		{
			m_fGoalRelevance = m_pGoalRecord->fIntrinsicRelevance;
			return;
		}
	}

	// Don't dodge while aim testing is turned off.

	else if( !m_pAI->GetAIBlackBoard()->GetBBUpdateTargetAim() )
	{
		m_fGoalRelevance = 0.f;
		return;
	}

	// Only dodge if AI is standing, or at a node.

	SAIWORLDSTATE_PROP* pProp = m_pAI->GetAIWorldState()->GetWSProp( kWSK_AtNode, m_pAI->m_hObject );
/**
	if( ( m_pAI->GetAIBlackBoard()->GetBBPosture() != kAP_POS_Stand ) &&
		( pProp->hWSValue == NULL ) )
	{
		m_fGoalRelevance = 0.f;
		return;
	}
**/

	// Fire a few shots before dodging.

	if( pProp->hWSValue != NULL )
	{
		double fStateChangeTime = m_pAI->GetAIBlackBoard()->GetBBStateChangeTime();
		if( g_pLTServer->GetTime() < fStateChangeTime + 3.f )
		{
			m_fGoalRelevance = 0.f;
			return;
		}
	}

	// Target is aiming at me.

	pProp = m_pAI->GetAIWorldState()->GetWSProp( kWSK_TargetIsAimingAtMe, m_pAI->m_hObject );
	if( pProp && pProp->bWSValue )
	{
		m_fGoalRelevance = m_pGoalRecord->fIntrinsicRelevance;
		return;
	}

	// HACK: This is a complete hack for DarkAlma.

	if( m_pAI->GetAIBlackBoard()->GetBBHandleTouch() == kTouch_Damage )
	{
		m_bAlwaysDodgeWhenShot = false;
	}

	// END HACK

	// AI was just shot.

	if( m_bAlwaysDodgeWhenShot )
	{
		CAIWMFact factQuery;
		factQuery.SetFactType(kFact_Damage);
		CAIWMFact* pFact = m_pAI->GetAIWorkingMemory()->FindWMFact( factQuery );
		if( pFact && DidDamage(m_pAI, pFact) )
		{
			double fCurTime = g_pLTServer->GetTime();
			if( fCurTime - pFact->GetUpdateTime() < 1.f )
			{
				m_fGoalRelevance = m_pGoalRecord->fIntrinsicRelevance;
				return;
			}
		}
	}

	// Target is not aiming at me.

	m_fGoalRelevance = 0.f;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalDodge::ActivateGoal
//
//	PURPOSE:	Activate the goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalDodge::ActivateGoal()
{
	super::ActivateGoal();

	m_pAI->GetAIBlackBoard()->SetBBUpdateTargetAim( false);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalDodge::DeactivateGoal
//
//	PURPOSE:	Deactivate the goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalDodge::DeactivateGoal()
{
	super::DeactivateGoal();
	m_pAI->GetAIBlackBoard()->SetBBUpdateTargetAim( true );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalDodge::SetWSSatisfaction
//
//	PURPOSE:	Set the WorldState satisfaction conditions.
//
// ----------------------------------------------------------------------- //

void CAIGoalDodge::SetWSSatisfaction( CAIWorldState& WorldState )
{
	WorldState.SetWSProp( kWSK_TargetIsAimingAtMe, m_pAI->m_hObject, kWST_bool, false );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalDodge::IsWSSatisfied
//
//	PURPOSE:	Return true if the world state satisfies the goal.
//
// ----------------------------------------------------------------------- //

bool CAIGoalDodge::IsWSSatisfied( CAIWorldState* pwsWorldState )
{
	// Goal is satisfied when target is no longer aiming at me.

	SAIWORLDSTATE_PROP* pProp = pwsWorldState->GetWSProp( kWSK_TargetIsAimingAtMe, m_pAI->m_hObject );
	if( pProp && !pProp->bWSValue )
	{
		return true;
	}

	// Target is still aiming at me.

	return false;
}


