// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalReactToFinishingMove.cpp
//
// PURPOSE : 
//
// CREATED : 3/17/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIGoalReactToFinishingMove.h"
#include "AIGoalMgr.h"
#include "AIWorldState.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Goal, CAIGoalReactToFinishingMove, kGoal_ReactToFinishingMove );

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalReactToFinishingMove::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAIGoalReactToFinishingMove::CAIGoalReactToFinishingMove()
{
}

CAIGoalReactToFinishingMove::~CAIGoalReactToFinishingMove()
{
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIGoalReactToFinishingMove::Save/Load
//              
//	PURPOSE:	Handle saving and restoring the CAIGoalReactToFinishingMove
//              
//----------------------------------------------------------------------------

void CAIGoalReactToFinishingMove::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);
}

void CAIGoalReactToFinishingMove::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIGoalReactToFinishingMove::CalculateGoalRelevance
//              
//	PURPOSE:	
//              
//----------------------------------------------------------------------------

void CAIGoalReactToFinishingMove::CalculateGoalRelevance()
{
	// Only (and always) relevent if the player has asked us to perform a finishing move.
	if ( m_pAI->GetAIBlackBoard()->GetBBFinishingSyncAction() )
	{
		m_fGoalRelevance = m_pGoalRecord->fIntrinsicRelevance;
		return;
	}

	// Goal is not relevant.
	m_fGoalRelevance = 0.0f;
	return;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIGoalReactToFinishingMove::SetWSSatisfaction
//              
//	PURPOSE:	
//              
//----------------------------------------------------------------------------

void CAIGoalReactToFinishingMove::SetWSSatisfaction( CAIWorldState& WorldState )
{
	WorldState.SetWSProp( kWSK_ReactedToWorldStateEvent, m_pAI->m_hObject, kWST_ENUM_AIWorldStateEvent, kWSE_FinishingMove );
}

