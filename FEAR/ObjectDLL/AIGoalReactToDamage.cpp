// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalReactToDamage.cpp
//
// PURPOSE : AIGoalReactToDamage class implementation
//
// CREATED : 11/11/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIGoalReactToDamage.h"
#include "AI.h"
#include "AIDB.h"
#include "AIWorkingMemory.h"
#include "AIUtils.h"
#include "AIGoalMgr.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Goal, CAIGoalReactToDamage, kGoal_ReactToDamage );


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalReactToDamage::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAIGoalReactToDamage::CAIGoalReactToDamage()
{
	m_fLastDamageTime = 0.f;
	m_fCurrentDamageTime = 0.f;
	m_eWorldStateEvent = kWSE_Damage;
	m_bAllowReplanWhenCurGoal = true;
	m_bPlayRecoilOnBuildPlanFailure = true;
}

CAIGoalReactToDamage::~CAIGoalReactToDamage()
{
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIGoalReactToDamage::Save/Load
//              
//	PURPOSE:	Handle saving and restoring the CAIGoalPatrol
//              
//----------------------------------------------------------------------------

void CAIGoalReactToDamage::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);

	SAVE_TIME(m_fCurrentDamageTime);
	SAVE_TIME(m_fLastDamageTime);
}

void CAIGoalReactToDamage::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);

	LOAD_TIME(m_fCurrentDamageTime);
	LOAD_TIME(m_fLastDamageTime);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalReactToDamage::CalculateGoalRelevance
//
//	PURPOSE:	Calculate the current goal relevance.
//
// ----------------------------------------------------------------------- //

void CAIGoalReactToDamage::CalculateGoalRelevance()
{
	// Goal is relevant if we have been damaged recently.

	CAIWMFact factQuery;
	factQuery.SetFactType(kFact_Damage);
	CAIWMFact* pFact = m_pAI->GetAIWorkingMemory()->FindWMFact( factQuery );
	if( pFact && DidDamage( m_pAI, pFact ) )
	{
		// If we were recently damaged, this is a relevant goal.  Allow 
		// responding actions to handle any lower level filtering
		// based on damage type or animation existence.

		if( pFact->GetUpdateTime() > m_fLastDamageTime )
		{
			m_fGoalRelevance = m_pGoalRecord->fIntrinsicRelevance;
			return;
		}
	}

	// We have not been damaged recently.

	m_fGoalRelevance = 0.f;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalReactToDamage::ReplanRequired
//
//	PURPOSE:	Return true if AI needs to make a new plan.
//
// ----------------------------------------------------------------------- //

bool CAIGoalReactToDamage::ReplanRequired()
{
	// Replan if this isn't the current goal, or if the goal is allowed
	// to replan while the goal is the current.

	if ( g_pAIDB->GetAIConstantsRecord()->bAllowRecoilsToInterruptRecoils )
	{
		CAIWMFact factQuery;
		factQuery.SetFactType(kFact_Damage);
		CAIWMFact* pFact = m_pAI->GetAIWorkingMemory()->FindWMFact( factQuery );
		if( pFact && DidDamage(m_pAI, pFact))
		{
			if( pFact->GetUpdateTime() > m_fCurrentDamageTime )
			{
				return true;
			}
		}
	}

	// Always handle damage while knocked out.
	// This is needed to allow one hit kills while knocked out.

	if( ( kAP_ATVT_KnockDown == m_pAI->GetAnimationContext()->GetCurrentProp( kAPG_Activity ) ) &&
		( !m_pAI->GetAnimationContext()->IsTransitioning() ) )
	{
		CAIWMFact factQuery;
		factQuery.SetFactType(kFact_Damage);
		CAIWMFact* pFact = m_pAI->GetAIWorkingMemory()->FindWMFact( factQuery );
		if( pFact && DidDamage(m_pAI, pFact))
		{
			if( pFact->GetUpdateTime() + 0.3f > g_pLTServer->GetTime() )
			{
				return true;
			}
		}
	}

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalReactToDamage::HandleBuildPlanFailure
//
//	PURPOSE:	Handle plan construction failing to find a valid plan to
//				accomplish this goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalReactToDamage::HandleBuildPlanFailure()
{
	super::HandleBuildPlanFailure();

	// Do not repeatedly try to react to the same damage.

	CAIWMFact factQuery;
	factQuery.SetFactType(kFact_Damage);
	CAIWMFact* pFact = m_pAI->GetAIWorkingMemory()->FindWMFact( factQuery );
	if( pFact && DidDamage(m_pAI, pFact))
	{
		m_fLastDamageTime = pFact->GetUpdateTime();
	}

	// Blend a short recoil if the hit was recent, as a default reaction.

	if( m_bPlayRecoilOnBuildPlanFailure
		&& ( !g_pAIDB->GetAIConstantsRecord()->bAllowRecoilsToInterruptRecoils )
		&& ( m_fLastDamageTime + 0.3f > g_pLTServer->GetTime() ) )
	{
		m_pAI->HandleShortRecoil();
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalReactToDamage::ActivateGoal
//
//	PURPOSE:	Activate the goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalReactToDamage::ActivateGoal()
{
	super::ActivateGoal();

	CAIWMFact factQuery;
	factQuery.SetFactType(kFact_Damage);
	CAIWMFact* pFact = m_pAI->GetAIWorkingMemory()->FindWMFact( factQuery );
	if( pFact && DidDamage(m_pAI, pFact) )
	{
		m_fCurrentDamageTime = pFact->GetUpdateTime();
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalReactToDamage::DeactivateGoal
//
//	PURPOSE:	Deactivate the goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalReactToDamage::DeactivateGoal()
{
	super::DeactivateGoal();

	m_fLastDamageTime = m_fCurrentDamageTime;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalReactToDamage::SetWSSatisfaction
//
//	PURPOSE:	Set the WorldState satisfaction conditions.
//
// ----------------------------------------------------------------------- //

void CAIGoalReactToDamage::SetWSSatisfaction( CAIWorldState& WorldState )
{
	WorldState.SetWSProp( kWSK_ReactedToWorldStateEvent, m_pAI->m_hObject, kWST_ENUM_AIWorldStateEvent, m_eWorldStateEvent );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalReactToDamage::IsWSSatisfied
//
//	PURPOSE:	Return true if the world state satisfies the goal.
//
// ----------------------------------------------------------------------- //

bool CAIGoalReactToDamage::IsWSSatisfied( CAIWorldState* pwsWorldState )
{
	// We have handled the damage.

	SAIWORLDSTATE_PROP* pProp = pwsWorldState->GetWSProp( kWSK_ReactedToWorldStateEvent, m_pAI->m_hObject );
	if( pProp && ( pProp->eAIWorldStateEventWSValue == m_eWorldStateEvent ) )
	{
		return true;
	}

	// We have not handled the damage.

	return false;
}


