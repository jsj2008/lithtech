// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalReactToWeaponBroke.cpp
//
// PURPOSE : 
//
// CREATED : 3/25/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIGoalReactToWeaponBroke.h"
#include "AIGoalMgr.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Goal, CAIGoalReactToWeaponBroke, kGoal_ReactToWeaponBroke );

// If the AIs weapon broke longer ago than this, it is too late to respond.
// Ideally, we would do some kind of 'cleanup' if the AI doesn't respond 
// based on something higher priority to do, but we only get planning 
// notifications when the AI replans.

static float gk_flWeaponBrokeTimeDelta = 0.1f;

static CAIWMFact* GetWeaponBrokeFact( CAI* pAI )
{
	CAIWMFact factWeaponBrokeQuery;
	factWeaponBrokeQuery.SetFactType( kFact_Knowledge );
	factWeaponBrokeQuery.SetKnowledgeType( kKnowledge_WeaponBroke );
	return pAI->GetAIWorkingMemory()->FindWMFact( factWeaponBrokeQuery );
}

static CAIWMFact* GetDamagedFact( CAI* pAI )
{
	CAIWMFact factDamagedQuery;
	factDamagedQuery.SetFactType( kFact_Damage );
	CAIWMFact* pDamagedFact = pAI->GetAIWorkingMemory()->FindWMFact( factDamagedQuery );
	if ( !DidDamage( pAI, pDamagedFact ) )
	{
		return NULL;
	}

	return pDamagedFact;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalReactToWeaponBroke::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAIGoalReactToWeaponBroke::CAIGoalReactToWeaponBroke()
{
}

CAIGoalReactToWeaponBroke::~CAIGoalReactToWeaponBroke()
{
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIGoalReactToWeaponBroke::Save/Load
//              
//	PURPOSE:	Handle saving and restoring the CAIGoalReactToWeaponBroke
//              
//----------------------------------------------------------------------------

void CAIGoalReactToWeaponBroke::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);
}

void CAIGoalReactToWeaponBroke::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIGoalReactToWeaponBroke::CalculateGoalRelevance
//              
//	PURPOSE:	Calculate the current goal relevance.
//              
//----------------------------------------------------------------------------

void CAIGoalReactToWeaponBroke::CalculateGoalRelevance()
{
	// AIs weapon didn't break.

	CAIWMFact* pWeaponBrokeFact = GetWeaponBrokeFact( m_pAI );
	if ( !pWeaponBrokeFact )
	{
		m_fGoalRelevance = 0.0f;
		return;
	}

	// AI took damage more recently.  Blocking should yield to damage 
	// response, even if it is currently executing.
	// NOTE: This assumes that the AI will always play a recoil animation.

	CAIWMFact* pDamagedFact = GetDamagedFact( m_pAI );
	if ( pDamagedFact 
		&& ( pDamagedFact->GetUpdateTime() > pWeaponBrokeFact->GetTime() ) )
	{
		m_fGoalRelevance = 0.0f;
		return;
	}

	// Relevant if the goal is currently executing and not satisfied.

	if ( m_pAI->GetGoalMgr()->IsCurGoal( this ) && !IsWSSatisfied( m_pAI->GetAIWorldState() ) )
	{
		m_fGoalRelevance = m_pGoalRecord->fIntrinsicRelevance;
		return;
	}

	// AI wasn't shoved recently enough to respond.

	if ( pWeaponBrokeFact->GetTime() < g_pLTServer->GetTime() - gk_flWeaponBrokeTimeDelta )
	{
		m_fGoalRelevance = 0.0f;
		return;
	}

	m_fGoalRelevance = m_pGoalRecord->fIntrinsicRelevance;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalReactToWeaponBroke::SetWSSatisfaction
//
//	PURPOSE:	Set the WorldState satisfaction conditions.
//
// ----------------------------------------------------------------------- //

void CAIGoalReactToWeaponBroke::SetWSSatisfaction( CAIWorldState& WorldState )
{
	WorldState.SetWSProp( kWSK_ReactedToWorldStateEvent, m_pAI->m_hObject, kWST_ENUM_AIWorldStateEvent, kWSE_WeaponBroke );
}
