// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalReactToBlockedPath.cpp
//
// PURPOSE : 
//
// CREATED : 10/25/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIGoalReactToBlockedPath.h"
#include "AIGoalMgr.h"
#include "AIWorldState.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Goal, CAIGoalReactToBlockedPath, kGoal_ReactToBlockedPath );

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalReactToBlockedPath::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAIGoalReactToBlockedPath::CAIGoalReactToBlockedPath()
{
}

CAIGoalReactToBlockedPath::~CAIGoalReactToBlockedPath()
{
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIGoalReactToBlockedPath::Save/Load
//              
//	PURPOSE:	Handle saving and restoring the CAIGoalReactToBlockedPath
//              
//----------------------------------------------------------------------------

void CAIGoalReactToBlockedPath::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);
}

void CAIGoalReactToBlockedPath::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIGoalReactToBlockedPath::CalculateGoalRelevance
//              
//	PURPOSE:	
//              
//----------------------------------------------------------------------------

void CAIGoalReactToBlockedPath::CalculateGoalRelevance()
{
	// Only relevant if the AI has a path set.

	if ( !m_pAI->GetGoalMgr()->IsCurGoal( this) &&
		kNav_Set != m_pAI->GetAIBlackBoard()->GetBBDestStatus(  ) )
	{
		m_fGoalRelevance = 0.0f;
		return;
	}

	// Only relevant if the AI knows that it is blocked.

	if ( 0 == ( kAIMovementFlag_BlockedPath & m_pAI->GetAIBlackBoard()->GetBBMovementCollisionFlags() ) )
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
//	ROUTINE:	CAIGoalReactToBlockedPath::SetWSSatisfaction
//              
//	PURPOSE:	
//              
//----------------------------------------------------------------------------

void CAIGoalReactToBlockedPath::SetWSSatisfaction( CAIWorldState& WorldState )
{
	WorldState.SetWSProp( kWSK_ReactedToWorldStateEvent, m_pAI->m_hObject, kWST_ENUM_AIWorldStateEvent, kWSE_BlockedPath );
}
