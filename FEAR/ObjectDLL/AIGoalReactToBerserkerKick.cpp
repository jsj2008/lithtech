// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalReactToBerserkerKick.cpp
//
// PURPOSE : 
//
// CREATED : 3/01/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIGoalReactToBerserkerKick.h"
#include "AIGoalMgr.h"
#include "AIWorldState.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Goal, CAIGoalReactToBerserkerKick, kGoal_ReactToBerserkerKick );

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalReactToBerserkerKick::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAIGoalReactToBerserkerKick::CAIGoalReactToBerserkerKick()
{
}

CAIGoalReactToBerserkerKick::~CAIGoalReactToBerserkerKick()
{
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIGoalReactToBerserkerKick::Save/Load
//              
//	PURPOSE:	Handle saving and restoring the CAIGoalReactToBerserkerKick
//              
//----------------------------------------------------------------------------

void CAIGoalReactToBerserkerKick::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);
}

void CAIGoalReactToBerserkerKick::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);
}

void CAIGoalReactToBerserkerKick::ActivateGoal()
{
	super::ActivateGoal();

	// Remove the fact once we have responded to it.

	CAIWMFact queryFact;
	queryFact.SetFactType( kFact_Knowledge );
	queryFact.SetKnowledgeType( kKnowledge_BerserkerKicked );
	m_pAI->GetAIWorkingMemory()->ClearWMFact( queryFact );
}


//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIGoalReactToBerserkerKick::CalculateGoalRelevance
//              
//	PURPOSE:	
//              
//----------------------------------------------------------------------------

void CAIGoalReactToBerserkerKick::CalculateGoalRelevance()
{
	// Only relevant if the AI is currenting performing a berserker attack.
	// (this is an optimization to avoid the working memory search every frame)
	// This prop is already required by AI.cpp for body state, so this does 
	// not create a new hardcoded dependency.

	if ( kAP_ACT_AttackBerserker != m_pAI->GetAnimationContext()->GetCurrentProp( kAPG_Action ) )
	{
		m_fGoalRelevance = 0.0f;
		return;
	}

	// Only relevent if the AI has a memory of being kicked recently.

	CAIWMFact queryFact;
	queryFact.SetFactType( kFact_Knowledge );
	queryFact.SetKnowledgeType( kKnowledge_BerserkerKicked );
	CAIWMFact* pFact = m_pAI->GetAIWorkingMemory()->FindWMFact( queryFact );
	if ( NULL == pFact )
	{
		m_fGoalRelevance = 0.0f;
		return;
	}

	// Goal is relevant.

	m_fGoalRelevance = m_pGoalRecord->fIntrinsicRelevance;
	return;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIGoalReactToBerserkerKick::SetWSSatisfaction
//              
//	PURPOSE:	
//              
//----------------------------------------------------------------------------

void CAIGoalReactToBerserkerKick::SetWSSatisfaction( CAIWorldState& WorldState )
{
	WorldState.SetWSProp( kWSK_ReactedToWorldStateEvent, m_pAI->m_hObject, kWST_ENUM_AIWorldStateEvent, kWSE_BerserkerKicked );
}
