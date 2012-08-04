// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalReactToMeleeBlocked.cpp
//
// PURPOSE : 
//
// CREATED : 9/14/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIGoalReactToMeleeBlocked.h"
#include "AIWorkingMemory.h"
#include "AIGoalMgr.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Goal, CAIGoalReactToMeleeBlocked, kGoal_Blocked );

// If the AI was blocked longer ago than this, it is too late to respond.  
// This should only occur if the AI was doing something more important than 
// blocking, a such as starting a long recoil.

static float gk_flBlockedTimeDelta = 0.1f;

static CAIWMFact* GetBlockedFact( CAI* pAI )
{
	CAIWMFact factBlockedQuery;
	factBlockedQuery.SetFactType( kFact_Knowledge );
	factBlockedQuery.SetKnowledgeType( kKnowledge_MeleeBlocked );
	return pAI->GetAIWorkingMemory()->FindWMFact( factBlockedQuery );
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
//	ROUTINE:	CAIGoalReactToMeleeBlocked::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAIGoalReactToMeleeBlocked::CAIGoalReactToMeleeBlocked()
{
}

CAIGoalReactToMeleeBlocked::~CAIGoalReactToMeleeBlocked()
{
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIGoalReactToMeleeBlocked::Save/Load
//              
//	PURPOSE:	Handle saving and restoring the CAIGoalReactToMeleeBlocked
//              
//----------------------------------------------------------------------------

void CAIGoalReactToMeleeBlocked::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);
}

void CAIGoalReactToMeleeBlocked::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalReactToMeleeBlocked::CalculateGoalRelevance
//
//	PURPOSE:	Calculate the current goal relevance.
//
// ----------------------------------------------------------------------- //

void CAIGoalReactToMeleeBlocked::CalculateGoalRelevance()
{
	// AI hasn't been blocked.

	CAIWMFact* pBlockedFact = GetBlockedFact( m_pAI );
	if ( !pBlockedFact )
	{
		m_fGoalRelevance = 0.0f;
		return;
	}

	// AI took damage more recently.  Blocking should yield to damage 
	// response, even if it is currently executing.
	// NOTE: This assumes that the AI will always play a recoil animation.

	CAIWMFact* pDamagedFact = GetDamagedFact( m_pAI );
	if ( pDamagedFact 
		&& ( pDamagedFact->GetUpdateTime() > pBlockedFact->GetTime() ) )
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

	// AI wasn't blocked recently enough to respond.

	if ( pBlockedFact->GetTime() < g_pLTServer->GetTime() - gk_flBlockedTimeDelta )
	{
		m_fGoalRelevance = 0.0f;
		return;
	}

	// AI was just blocked and should respond.

	m_fGoalRelevance = m_pGoalRecord->fIntrinsicRelevance;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalReactToMeleeBlocked::SetWSSatisfaction
//
//	PURPOSE:	Set the WorldState satisfaction conditions.
//
// ----------------------------------------------------------------------- //

void CAIGoalReactToMeleeBlocked::SetWSSatisfaction( CAIWorldState& WorldState )
{
	WorldState.SetWSProp( kWSK_ReactedToWorldStateEvent, m_pAI->m_hObject, kWST_ENUM_AIWorldStateEvent, kWSE_MeleeBlocked );
}
