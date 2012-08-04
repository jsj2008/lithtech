// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalReactToShove.cpp
//
// PURPOSE : 
//
// CREATED : 11/11/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIGoalReactToShove.h"
#include "AIGoalMgr.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Goal, CAIGoalReactToShove, kGoal_ReactToShove );

// If the AI was shoved longer ago than this, it is too late to respond.  
// This should only occur if the AI was doing something more important than 
// responding to being shoved, such as responding to a player action with a 
// recoil

static float gk_flShovedTimeDelta = 0.1f;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalReactToShove::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAIGoalReactToShove::CAIGoalReactToShove()
{
}

CAIGoalReactToShove::~CAIGoalReactToShove()
{
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIGoalReactToShove::Save/Load
//              
//	PURPOSE:	Handle saving and restoring the CAIGoalReactToShove
//              
//----------------------------------------------------------------------------

void CAIGoalReactToShove::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);
}

void CAIGoalReactToShove::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIGoalReactToShove::CalculateGoalRelevance
//              
//	PURPOSE:	Calculate the current goal relevance.
//              
//----------------------------------------------------------------------------

void CAIGoalReactToShove::CalculateGoalRelevance()
{
	// Relevant if the goal is currently executing and not satisfied.

	if ( m_pAI->GetGoalMgr()->IsCurGoal( this ) && !IsWSSatisfied( m_pAI->GetAIWorldState() ) )
	{
		m_fGoalRelevance = m_pGoalRecord->fIntrinsicRelevance;
		return;
	}

	// AI hasn't been shoved.

	CAIWMFact factQuery;
	factQuery.SetFactType( kFact_Knowledge );
	factQuery.SetKnowledgeType( kKnowledge_Shoved );

	CAIWMFact* pFact = m_pAI->GetAIWorkingMemory()->FindWMFact( factQuery );
	if ( !pFact )
	{
		m_fGoalRelevance = 0.0f;
		return;
	}

	// AI wasn't shoved recently enough to respond.

	if ( pFact->GetTime() < g_pLTServer->GetTime() - gk_flShovedTimeDelta )
	{
		m_fGoalRelevance = 0.0f;
		return;
	}

	// AI was just shoved and should respond.

	m_fGoalRelevance = m_pGoalRecord->fIntrinsicRelevance;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalReactToShove::SetWSSatisfaction
//
//	PURPOSE:	Set the WorldState satisfaction conditions.
//
// ----------------------------------------------------------------------- //

void CAIGoalReactToShove::SetWSSatisfaction( CAIWorldState& WorldState )
{
	WorldState.SetWSProp( kWSK_ReactedToWorldStateEvent, m_pAI->m_hObject, kWST_ENUM_AIWorldStateEvent, kWSE_Shoved );
}
