// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalRetreat.cpp
//
// PURPOSE : AIGoalRetreat class implementation
//
// CREATED : 4/24/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIGoalRetreat.h"
#include "AI.h"
#include "AINode.h"
#include "AIBlackBoard.h"
#include "AIWorkingMemory.h"
#include "AIUtils.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Goal, CAIGoalRetreat, kGoal_Retreat );


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalRetreat::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAIGoalRetreat::CAIGoalRetreat()
{
	m_dwNodeStatus = kNodeStatus_All & ~kNodeStatus_ThreatBlockingPath;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalRetreat::CalculateGoalRelevance
//
//	PURPOSE:	Calculate the current goal relevance.
//
// ----------------------------------------------------------------------- //

void CAIGoalRetreat::CalculateGoalRelevance()
{
	// We may be at some node.

	HOBJECT hNode = NULL;
	SAIWORLDSTATE_PROP* pProp = m_pAI->GetAIWorldState()->GetWSProp( kWSK_AtNode, m_pAI->m_hObject );
	if( pProp )
	{
		hNode = pProp->hWSValue;
	}

	// Goal has no relevance if we are already at a valid node, and this goal is not currently active.

	if( hNode && !m_hNode )
	{
		AINode* pNode = (AINode*)g_pLTServer->HandleToObject( hNode );
		if( pNode && pNode->IsNodeValid( m_pAI, m_pAI->GetPosition(), m_pAI->GetAIBlackBoard()->GetBBTargetObject(), kThreatPos_TargetPos, kNodeStatus_All ) )
		{
			m_fGoalRelevance = 0.f;
			return;
		}
	}

	// Goal has no relevance if we are already at an invalid node.

	if( hNode )
	{
		AINode* pNode = (AINode*)g_pLTServer->HandleToObject( hNode );
		if( pNode && ( !pNode->IsNodeValid( m_pAI, m_pAI->GetPosition(), m_pAI->GetAIBlackBoard()->GetBBTargetObject(), kThreatPos_TargetPos, kNodeStatus_All ) ) )
		{
			m_fGoalRelevance = 0.f;
			return;
		}
	}

	// Goal is only relevant if we have the desire to retreat.

	CAIWMFact factQuery;
	factQuery.SetFactType(kFact_Desire);
	factQuery.SetDesireType(kDesire_Retreat);
	CAIWMFact* pFact = m_pAI->GetAIWorkingMemory()->FindWMFact( factQuery );
	if( !pFact )
	{
		return;
	}

	// Default calculation.

	super::CalculateGoalRelevance();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalRetreat::FindBestNode
//
//	PURPOSE:	Return the best available node.
//
// ----------------------------------------------------------------------- //

HOBJECT CAIGoalRetreat::FindBestNode( EnumAINodeType eNodeType )
{
	HOBJECT hThreatObject = NULL;
	if( m_pAI->HasTarget( kTarget_Character | kTarget_Object ) )
	{
		hThreatObject = m_pAI->GetAIBlackBoard()->GetBBTargetObject();
	}

	if (hThreatObject)
	{
		CAIWMFact* pFact = m_pAI->GetAIWorkingMemory()->FindFactNodeRandom( m_pAI, eNodeType, m_dwNodeStatus, m_hNode, hThreatObject );
		if( pFact )
		{
			return pFact->GetTargetObject();
		}
	}

	return NULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalRetreat::DeactivateGoal
//
//	PURPOSE:	Deactivate the goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalRetreat::DeactivateGoal()
{
	// Clear the desire to retreat.

	SAIWORLDSTATE_PROP* pProp = m_pAI->GetAIWorldState()->GetWSProp( kWSK_AtNode, m_pAI->m_hObject );
	if( pProp && ( pProp->hWSValue == m_hNode ) )
	{
		CAIWMFact factQuery;
		factQuery.SetFactType(kFact_Desire);
		factQuery.SetDesireType(kDesire_Retreat);
		m_pAI->GetAIWorkingMemory()->ClearWMFacts( factQuery );
	}

	// Also clear any cover or ambush tasks.

	CAIWMFact factTaskQuery;
	factTaskQuery.SetFactType(kFact_Task);
	m_pAI->GetAIWorkingMemory()->ClearWMFacts( factTaskQuery );

	// Default behavior.

	super::DeactivateGoal();
}
