// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalFollowWait.cpp
//
// PURPOSE : AIGoalFollowWait class implementation
//
// CREATED : 07/16/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIGoalFollowWait.h"
#include "AI.h"
#include "AINodeFollow.h"
#include "AITarget.h"
#include "AIWorkingMemory.h"


DEFINE_AI_FACTORY_CLASS_SPECIFIC( Goal, CAIGoalFollowWait, kGoal_FollowWait );


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalFollowWait::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAIGoalFollowWait::CAIGoalFollowWait()
{
	m_hFollowNode = NULL;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIGoalFollowWait::Save/Load
//              
//	PURPOSE:	Handle saving and restoring the CAIGoal
//              
//----------------------------------------------------------------------------

void CAIGoalFollowWait::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);
	SAVE_HOBJECT( m_hFollowNode );
}

void CAIGoalFollowWait::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);
	LOAD_HOBJECT( m_hFollowNode );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalFollowWait::CalculateGoalRelevance
//
//	PURPOSE:	Calculate the current goal relevance.
//
// ----------------------------------------------------------------------- //

void CAIGoalFollowWait::CalculateGoalRelevance()
{
	// Intentionally do NOT call super::CalculateGoalRelevance.
	// Base relevance off follow node relative to the leader.

	// Bail if we are targeting something other than the leader.

	if( !( m_pAI->HasTarget( kTarget_Leader ) ||
		   m_pAI->HasTarget( kTarget_Interest ) ) )
	{
		m_fGoalRelevance = 0.f;
		return;
	}

	// Bail if we have no Follow task.

	CAIWMFact factQuery;
	factQuery.SetFactType( kFact_Task );
	factQuery.SetTaskType( kTask_Follow );
	CAIWMFact* pFact = m_pAI->GetAIWorkingMemory()->FindWMFact( factQuery );
	if( !pFact )
	{
		m_fGoalRelevance = 0.f;
		return;
	}

	// Default behavior.

	// Goal is in progress.

	HOBJECT hLeader = pFact->GetSourceObject();
	if( IsGoalInProgress( hLeader ) )
	{
		m_fGoalRelevance = m_pGoalRecord->fIntrinsicRelevance;
		return;
	}

	// Goal is relevant if we know of available nodes.

	m_hFollowNode = NULL;
	m_hNodeBest = FindBestNode( m_pGoalRecord->eNodeType, hLeader, &m_hFollowNode );
	if( m_hNodeBest )
	{
		m_fGoalRelevance = m_pGoalRecord->fIntrinsicRelevance;
		return;
	}

	m_fGoalRelevance = 0.f;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalFollowWait::IsGoalInProgress
//
//	PURPOSE:	Return true if goal is already in progress.
//
// ----------------------------------------------------------------------- //

bool CAIGoalFollowWait::IsGoalInProgress( HOBJECT hLeader )
{
	// AI is not following anyone.

	if( !hLeader )
	{
		return false;
	}

	// Validate the waiting node.

	if( !super::IsGoalInProgress() )
	{
		return false;
	}

	// Validate the follow node.

	// No current node.

	if( !m_hFollowNode )
	{
		return false;
	}

	// Node is disabled.

	AINode* pNode = (AINode*)g_pLTServer->HandleToObject( m_hFollowNode );
	if( pNode && pNode->IsNodeDisabled() )
	{
		return false;
	}

	// Node no longer contains leader.

	if( pNode && !pNode->IsCharacterInRadiusOrRegion( hLeader ) )
	{
		return false;
	}

	// We are not yet at the node.

	SAIWORLDSTATE_PROP* pProp = m_pAI->GetAIWorldState()->GetWSProp( kWSK_AtNode, m_pAI->m_hObject );
	if( !( pProp && pProp->hWSValue == m_hNodeBest ) )
	{
		// The Player is standing on the destination node.

		AINode* pNodeBest = (AINode*)g_pLTServer->HandleToObject( m_hNodeBest );
		if( pNodeBest && !pNodeBest->IsNodeValid( m_pAI, m_pAI->GetPosition(), NULL, kThreatPos_TargetPos, kNodeStatus_PlayerOnNode ) )
		{
			return false;
		}
	}

	// Goal is still in progress.

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalFollowWait::FindBestNode
//
//	PURPOSE:	Return the best available node.
//
// ----------------------------------------------------------------------- //

HOBJECT CAIGoalFollowWait::FindBestNode( EnumAINodeType eNodeType, HOBJECT hLeader, LTObjRef* phFollowNode )
{
	// Sanity check.

	if( !( hLeader && phFollowNode ) )
	{
		*phFollowNode = NULL;
		return NULL;
	}

	HOBJECT hNodeWait = NULL;
	while( !hNodeWait )
	{
		// Bail if no follow nodes.

		CAIWMFact* pFact = m_pAI->GetAIWorkingMemory()->FindFactNodeMax( m_pAI, eNodeType, m_dwNodeStatus, m_hNode, NULL );
		if( !pFact )
		{
			*phFollowNode = NULL;
			return NULL;
		}

		// Bail if we have exhausted all of the known follow nodes.

		if( pFact->GetConfidence( CAIWMFact::kFactMask_Position ) == 0.f )
		{
			*phFollowNode = NULL;
			return NULL;
		}

		// Ignore fact if node doesn't exist.

		*phFollowNode = pFact->GetTargetObject();
		AINodeFollow* pNodeFollow = (AINodeFollow*)g_pLTServer->HandleToObject( *phFollowNode );
		if( !pNodeFollow )
		{
			pFact->SetConfidence( CAIWMFact::kFactMask_Position, 0.f );
			continue;
		}

		// Find a valid waiting node listed in the follow node.

		hNodeWait = FindValidWaitNode( pNodeFollow->GetWaitingNodes(), hLeader );

		// Ignore follow nodes that have no valid waiting nodes.

		if( !hNodeWait )
		{
			pFact->SetConfidence( CAIWMFact::kFactMask_Position, 0.f );
			continue;
		}
	}

	return hNodeWait;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalFollowWait::FindValidWaitNode
//
//	PURPOSE:	Return a valid waiting node.
//
// ----------------------------------------------------------------------- //

HOBJECT CAIGoalFollowWait::FindValidWaitNode( CNamedObjectList* plstWaitingNodes, HOBJECT hLeader )
{
	// Sanity check.

	if( !( plstWaitingNodes && hLeader ) )
	{
		return NULL;
	}

	const HOBJECT_LIST*	pNodeHandles = plstWaitingNodes->GetObjectHandles();

	HOBJECT hNode;
	AINode* pNode;
	HOBJECT_LIST::const_iterator itNode;
	for( itNode = pNodeHandles->begin(); itNode != pNodeHandles->end(); ++itNode )
	{
		// Skip objects that are not nodes.

		hNode = *itNode;
		if( !IsAINode( hNode ) )
		{
			continue;
		}
		pNode = (AINode*)g_pLTServer->HandleToObject( hNode );

		// Skip nodes that are not cover nodes.

		if( pNode->GetType() != kNode_Cover )
		{
			continue;
		}

		// Skip unavailable nodes.

		if( pNode->IsLockedDisabledOrTimedOut( m_pAI->GetHOBJECT() ) )
		{
			continue;
		}

		// Skip nodes that the Player is standing on.

		if( !pNode->IsNodeValid( m_pAI, m_pAI->GetPosition(), NULL, kThreatPos_TargetPos, kNodeStatus_PlayerOnNode ) )
		{
			continue;
		}

		// Do not select a node that requires passing the leader to reach.

		LTVector vLeaderPos;
		g_pLTServer->GetObjectPos( hLeader, &vLeaderPos );
		LTVector vToNode = pNode->GetPos() - vLeaderPos;
		LTVector vToAI = m_pAI->GetPosition() - vLeaderPos;
		if( vToNode.Dot( vToAI ) < 0.f )
		{
			continue; 
		}

		// Found a valid waiting node.

		return hNode;
	}

	// No valid waiting nodes.

	return NULL;
}
