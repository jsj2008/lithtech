// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalExitLink.cpp
//
// PURPOSE : 
//
// CREATED : 4/29/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIGoalExitLink.h"
#include "AINavMeshLinkAbstract.h"
#include "AINavMeshLinkExit.h"
#include "AIWorldState.h"
#include "AI.h"
#include "AIBlackBoard.h"
#include "AINode.h"
#include "AINavMesh.h"
#include "AIDB.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Goal, CAIGoalExitLink, kGoal_ExitLink );

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalExitLink::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAIGoalExitLink::CAIGoalExitLink()
{
}

CAIGoalExitLink::~CAIGoalExitLink()
{
}

void CAIGoalExitLink::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);
	SAVE_HOBJECT(m_hNode);
	SAVE_INT(m_ePendingNavMeshLink);
	SAVE_HOBJECT(m_hPendingNode);
}

void CAIGoalExitLink::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);
	LOAD_HOBJECT(m_hNode);
	LOAD_INT_CAST(m_ePendingNavMeshLink, ENUM_NMLinkID);
	LOAD_HOBJECT(m_hPendingNode);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalExitLink::CalculateGoalRelevance
//
//	PURPOSE:	Calculate the current goal relevance.
//
// ----------------------------------------------------------------------- //

void CAIGoalExitLink::CalculateGoalRelevance()
{
	// If the goal is already running, do not update the relevance.

	if (IsGoalInProgress())
	{
		m_fGoalRelevance = m_pGoalRecord->fIntrinsicRelevance;
		return;
	};

	m_fGoalRelevance = 0.f;

	// AI is not in a link.

	if ( kNMLink_Invalid == m_pAI->GetAIBlackBoard()->GetBBNextNMLink() )
	{
		return;
	}

	// Link does not exist.

	AINavMeshLinkAbstract* pLink = g_pAINavMesh->GetNMLink( m_pAI->GetAIBlackBoard()->GetBBNextNMLink() );
	if( !pLink )
	{
		return;
	}

	// Link is inactive.

	if( !pLink->IsNMLinkActiveToAI( m_pAI ) )
	{
		return;
	}

	// Link is not an exit link

	if ( kLink_Exit != pLink->GetNMLinkType())
	{
		return;
	}
	AINavMeshLinkExit* pExitLink = (AINavMeshLinkExit*)pLink;

	// Link is not relevant.

	if( !pExitLink->IsLinkExitRelevant( m_pAI ) )
	{
		return;
	}

	// No smartobject node to use.

	AINodeSmartObject* pNode = pExitLink->GetActionSmartObjectNode(m_pAI);
	if (!pNode)
	{
		return;
	}

	// Node is not valid.

	if (!pNode->IsNodeValid(m_pAI, m_pAI->GetPosition(), m_pAI->GetAIBlackBoard()->GetBBTargetObject(), kThreatPos_TargetPos, kNodeStatus_All))
	{
		return;
	}

	// AI may handle exiting the link.  Cache some info in case this is applied.

	m_fGoalRelevance = m_pGoalRecord->fIntrinsicRelevance;
	m_ePendingNavMeshLink = m_pAI->GetAIBlackBoard()->GetBBNextNMLink();
	m_hPendingNode = pNode->GetHOBJECT();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalExitLink::ActivateGoal
//
//	PURPOSE:	Activate the goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalExitLink::ActivateGoal()
{
	super::ActivateGoal();
	m_hNode = m_hPendingNode;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalExitLink::DeactivateGoal
//
//	PURPOSE:	Deactivate the goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalExitLink::DeactivateGoal()
{
	super::DeactivateGoal();

	m_hNode = NULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalExitLink::SetWSSatisfaction
//
//	PURPOSE:	Set the WorldState satisfaction conditions.
//
// ----------------------------------------------------------------------- //

void CAIGoalExitLink::SetWSSatisfaction( CAIWorldState& WorldState )
{
	// Set satisfaction properties.

	WorldState.SetWSProp( kWSK_UsingObject, m_pAI->m_hObject, kWST_HOBJECT, m_hPendingNode );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalExitLink::IsWSSatisfied
//
//	PURPOSE:	Return true if the world state satisfies the goal.
//
// ----------------------------------------------------------------------- //

bool CAIGoalExitLink::IsWSSatisfied( CAIWorldState* pwsWorldState )
{
	SAIWORLDSTATE_PROP* pProp = pwsWorldState->GetWSProp( kWSK_UsingObject, m_pAI->m_hObject );
	if( pProp && pProp->hWSValue && ( pProp->hWSValue == m_hNode ) )
	{
		return true;
	}

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalExitLink::IsGoalInProgress
//
//	PURPOSE:	Return true if goal is already in progress.
//
// ----------------------------------------------------------------------- //

bool CAIGoalExitLink::IsGoalInProgress()
{
	if( m_hNode && !IsWSSatisfied( m_pAI->GetAIWorldState() ) )
	{
		return true;
	}

	return false;
}
