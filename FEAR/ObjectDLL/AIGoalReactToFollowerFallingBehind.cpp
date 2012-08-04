// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalReactToFollowerFallingBehind.cpp
//
// PURPOSE : 
//
// CREATED : 4/07/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIGoalReactToFollowerFallingBehind.h"
#include "AINodeLead.h"
#include "AIGoalMgr.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Goal, CAIGoalReactToFollowerFallingBehind, kGoal_ReactToFollowerDistanceChange );

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalReactToFollowerFallingBehind::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAIGoalReactToFollowerFallingBehind::CAIGoalReactToFollowerFallingBehind()
{
}

CAIGoalReactToFollowerFallingBehind::~CAIGoalReactToFollowerFallingBehind()
{
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIGoalReactToFollowerFallingBehind::Save/Load
//              
//	PURPOSE:	Handle saving and restoring the CAIGoalReactToFollowerFallingBehind
//              
//----------------------------------------------------------------------------

void CAIGoalReactToFollowerFallingBehind::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);
}

void CAIGoalReactToFollowerFallingBehind::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIGoalReactToFollowerFallingBehind::CalculateGoalRelevance
//              
//	PURPOSE:	Calculate the current goal relevance.
//              
//----------------------------------------------------------------------------

void CAIGoalReactToFollowerFallingBehind::CalculateGoalRelevance()
{
	// If this goal is currently active, and if it is not satisfied, continue
	// executing.

	if ( m_pAI->GetGoalMgr()->IsCurGoal( this ) 
		&& !IsWSSatisfied( m_pAI->GetAIWorldState() ) 
		&& IsPlanValid() )
	{
		m_fGoalRelevance = m_pGoalRecord->fIntrinsicRelevance;
		return;
	}
	
	// Not relevant unless the AI is targeting a follower.

	if ( false == m_pAI->HasTarget( kTarget_Follower ) )
	{
		m_fGoalRelevance = 0.0f;
		return;
	}

	CAIWMFact factQuery;
	factQuery.SetFactType( kFact_Task );
	factQuery.SetTaskType( kTask_LeadCharacter );
	factQuery.SetSourceObject( m_pAI->GetAIBlackBoard()->GetBBTargetObject() );
	CAIWMFact* pFact = m_pAI->GetAIWorkingMemory()->FindWMFact( factQuery );
	if( !pFact )
	{
		m_fGoalRelevance = 0.0f;
		return;
	}

	// Not relevant if the AI doesn't have a destination or follower.

	AINodeLead* pDestination = AINodeLead::DynamicCast( pFact->GetTargetObject() );
	HOBJECT hCharacter = pFact->GetSourceObject();

	if ( IsDeadCharacter( hCharacter)
		|| NULL == pDestination )
	{
		m_fGoalRelevance = 0.0f;
		return;
	}

	// Not relevant if the follower is inside the radius specified by the 
	// destination node.

	LTVector vFollowerPos;
	g_pLTServer->GetObjectPos( hCharacter, &vFollowerPos );
	float flDistanceToFollowerSqr = ( ( m_pAI->GetPosition() - vFollowerPos ) ).MagSqr();
	if ( pDestination->GetStartWaitingRadiusSqr() > flDistanceToFollowerSqr )
	{
		m_fGoalRelevance = 0.0f;
		return;
	}

	// Success!

	m_fGoalRelevance = m_pGoalRecord->fIntrinsicRelevance;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIGoalReactToFollowerFallingBehind::SetWSSatisfaction
//              
//	PURPOSE:	Set the WorldState satisfaction conditions.
//              
//----------------------------------------------------------------------------

void CAIGoalReactToFollowerFallingBehind::SetWSSatisfaction( CAIWorldState& WorldState )
{
	WorldState.SetWSProp( kWSK_ReactedToWorldStateEvent, m_pAI->m_hObject, kWST_ENUM_AIWorldStateEvent, kWSE_FollowerOutOfRange );
}
