// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalTraverseLink.cpp
//
// PURPOSE : AIGoalTraverseLink class implementation
//
// CREATED : 7/24/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIGoalTraverseLink.h"
#include "AI.h"
#include "AIDB.h"
#include "AIBlackBoard.h"
#include "AINavMesh.h"
#include "AINavMeshLinkAbstract.h"
#include "AINavigationMgr.h"
#include "AIPathKnowledgeMgr.h"
#include "AIGoalMgr.h"
#include "AIUtils.h"


DEFINE_AI_FACTORY_CLASS_SPECIFIC( Goal, CAIGoalTraverseLink, kGoal_TraverseLink );


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalTraverseLink::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAIGoalTraverseLink::CAIGoalTraverseLink()
{
	m_eTraversingLink = kNMLink_Invalid;
}

CAIGoalTraverseLink::~CAIGoalTraverseLink()
{
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIGoalTraverseLink::Save/Load
//              
//	PURPOSE:	Handle saving and restoring the CAIGoalGoto
//              
//----------------------------------------------------------------------------

void CAIGoalTraverseLink::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);

	SAVE_DWORD( m_eTraversingLink );
}

void CAIGoalTraverseLink::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);

	LOAD_DWORD_CAST( m_eTraversingLink, ENUM_NMLinkID );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalTraverseLink::CalculateGoalRelevance
//
//	PURPOSE:	Calculate the current goal relevance.
//
// ----------------------------------------------------------------------- //

void CAIGoalTraverseLink::CalculateGoalRelevance()
{
	m_fGoalRelevance = 0.f;

	// AI is in the process of traversing this link.

	if( m_pAI->GetGoalMgr()->IsCurGoal( this ) )
	{
		AINavMeshLinkAbstract* pLink = g_pAINavMesh->GetNMLink( m_eTraversingLink );
		if( pLink && pLink->IsTraversalInProgress( m_pAI ) )
		{
			m_fGoalRelevance = m_pGoalRecord->fIntrinsicRelevance;
			return;
		}
	}

	// No path set.

	if( m_pAI->GetAIBlackBoard()->GetBBDestStatus() != kNav_Set )
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

	// Link is not relevant.

	if( !pLink->IsLinkRelevant( m_pAI ) )
	{
		return;
	}

	// AI needs to handle traversing the link.

	m_fGoalRelevance = m_pGoalRecord->fIntrinsicRelevance;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalTraverseLink::ActivateGoal
//
//	PURPOSE:	Activate the goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalTraverseLink::ActivateGoal()
{
	super::ActivateGoal();

	m_eTraversingLink = m_pAI->GetAIBlackBoard()->GetBBNextNMLink();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalTraverseLink::DeactivateGoal
//
//	PURPOSE:	Deactivate the goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalTraverseLink::DeactivateGoal()
{
	super::DeactivateGoal();

	// Cleanup the traversal.

	AINavMeshLinkAbstract* pLink = g_pAINavMesh->GetNMLink( m_eTraversingLink );
	if( pLink )
	{
		pLink->DeactivateTraversal( m_pAI );
	}

	m_eTraversingLink = kNMLink_Invalid;

	// Clear any cached pathfinding data, because the AI may have
	// landed somewhere disconnected from where he started.

	m_pAI->GetPathKnowledgeMgr()->ClearPathKnowledge();

	// Clear last traversed link.

	m_pAI->GetAIWorldState()->SetWSProp( kWSK_TraversedLink, m_pAI->m_hObject, kWST_ENUM_NMLinkID, kNMLink_Invalid );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalTraverseLink::SetWSSatisfaction
//
//	PURPOSE:	Set the WorldState satisfaction conditions.
//
// ----------------------------------------------------------------------- //

void CAIGoalTraverseLink::SetWSSatisfaction( CAIWorldState& WorldState )
{
	WorldState.SetWSProp( kWSK_TraversedLink, m_pAI->m_hObject, kWST_ENUM_NMLinkID, m_pAI->GetAIBlackBoard()->GetBBNextNMLink() );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalTraverseLink::IsWSSatisfied
//
//	PURPOSE:	Return true if the world state satisfies the goal.
//
// ----------------------------------------------------------------------- //

bool CAIGoalTraverseLink::IsWSSatisfied( CAIWorldState* pwsWorldState )
{
	ENUM_NMLinkID eLink = m_pAI->GetAIBlackBoard()->GetBBNextNMLink();
	if( eLink == kNMLink_Invalid )
	{
		return true;
	}

	// The goal is satisfied when the AI has traversed the link.

	SAIWORLDSTATE_PROP* pProp = pwsWorldState->GetWSProp( kWSK_TraversedLink, m_pAI->m_hObject );
	if( pProp &&
		( eLink == pProp->eNMLinkIDWSValue ) )
	{
		return true;
	}

	return false;
}


