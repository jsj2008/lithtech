// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalKillEnemy.cpp
//
// PURPOSE : AIGoalKillEnemy class implementation
//
// CREATED : 1/28/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIGoalKillEnemy.h"
#include "AI.h"
#include "AIDB.h"
#include "AIPlanner.h"
#include "AITarget.h"
#include "AIBlackBoard.h"
#include "AIWorkingMemory.h"


DEFINE_AI_FACTORY_CLASS_SPECIFIC( Goal, CAIGoalKillEnemy, kGoal_KillEnemy );


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalKillEnemy::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAIGoalKillEnemy::CAIGoalKillEnemy()
{
}

CAIGoalKillEnemy::~CAIGoalKillEnemy()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalKillEnemy::CalculateGoalRelevance
//
//	PURPOSE:	Calculate the current goal relevance.
//
// ----------------------------------------------------------------------- //

void CAIGoalKillEnemy::CalculateGoalRelevance()
{
	// AI is aware of an enemy.

	if( m_pAI->HasTarget( kTarget_Character | kTarget_Object ) )
	{
		m_fGoalRelevance = m_pGoalRecord->fIntrinsicRelevance;
		return;
	}

	// No enemy present.

	m_fGoalRelevance = 0.f;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalKillEnemy::ReplanRequired
//
//	PURPOSE:	Return true if AI needs to make a new plan.
//
// ----------------------------------------------------------------------- //

bool CAIGoalKillEnemy::ReplanRequired()
{
	// Replan if we have been damaged since the last plan activation.
	// This allows a soldier to switch from cover fire to blind fire.
	// Do not replan in the middle fo a locked attack anim (e.g. a lunge).

	CAIPlan* pPlan = m_pAI->GetAIPlan();
	if( pPlan && !m_pAI->GetAnimationContext()->IsLocked() )
	{
		CAIWMFact factQuery;
		factQuery.SetFactType(kFact_Damage);
		CAIWMFact* pFact = m_pAI->GetAIWorkingMemory()->FindWMFact( factQuery );
		if( pFact 
			&& ( DidDamage(m_pAI, pFact) )
			&& ( pFact->GetUpdateTime() > pPlan->GetPlanActivationTime() ) )
		{
			return true;
		}
	}

	// Replan if any weapon statuses have changed since this goal activated.

	if( !m_pAI->GetAnimationContext()->IsLocked() )
	{
		double fTime;
		for( uint32 iWeaponType = 0; iWeaponType < kAIWeaponType_Count; ++iWeaponType )
		{
			fTime = m_pAI->GetAIBlackBoard()->GetBBWeaponStatusChangeTime( (ENUM_AIWeaponType)iWeaponType );
			if( fTime > m_fActivationTime )
			{
				return true;
			}
		}
	}

	// No replan required.

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalKillEnemy::ActivateGoal
//
//	PURPOSE:	Activate the goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalKillEnemy::ActivateGoal()
{
	super::ActivateGoal();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalKillEnemy::SetWSSatisfaction
//
//	PURPOSE:	Set the WorldState satisfaction conditions.
//
// ----------------------------------------------------------------------- //

void CAIGoalKillEnemy::SetWSSatisfaction( CAIWorldState& WorldState )
{
	WorldState.SetWSProp( kWSK_TargetIsDead, m_pAI->m_hObject, kWST_bool, true );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalKillEnemy::IsWSSatisfied
//
//	PURPOSE:	Return true if the world state satisfies the goal.
//
// ----------------------------------------------------------------------- //

bool CAIGoalKillEnemy::IsWSSatisfied( CAIWorldState* pwsWorldState )
{
	SAIWORLDSTATE_PROP* pProp = pwsWorldState->GetWSProp( kWSK_TargetIsDead, m_pAI->m_hObject );
	if( pProp && ( pProp->bWSValue ) )
	{
		return true;
	}

	return false;
}

