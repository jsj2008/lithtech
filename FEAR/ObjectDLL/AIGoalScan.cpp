// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalScan.cpp
//
// PURPOSE : AIGoalScan class implementation
//
// CREATED : 9/18/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIGoalScan.h"
#include "AINodeScanner.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Goal, CAIGoalScan, kGoal_Scan );


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalScan::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAIGoalScan::CAIGoalScan()
{
}

CAIGoalScan::~CAIGoalScan()
{
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIGoalScan::Save/Load
//              
//	PURPOSE:	Handle saving and restoring the CAIGoalPatrol
//              
//----------------------------------------------------------------------------

void CAIGoalScan::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);
}

void CAIGoalScan::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalScan::ActivateGoal
//
//	PURPOSE:	Activate the goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalScan::ActivateGoal()
{
	super::ActivateGoal();

	// Start scanning with a clean slate.

	m_pAI->GetAIBlackBoard()->SetBBTargetedTypeMask( kTarget_None );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalScan::DeactivateGoal
//
//	PURPOSE:	Deactivate the goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalScan::DeactivateGoal()
{
	super::DeactivateGoal();

	// Clear known disturbances.

	CAIWMFact factQuery;
	factQuery.SetFactType( kFact_Disturbance );
	m_pAI->GetAIWorkingMemory()->ClearWMFacts( factQuery );

	// Run command upon arrival at default node.

	if( ( !m_pAI->GetAIBlackBoard()->GetBBSensesOn() ) && 
		( m_pAI->GetAIBlackBoard()->GetBBDestStatus() == kNav_Done ) )
	{
		AINodeScanner* pNode = (AINodeScanner*)g_pLTServer->HandleToObject( m_hNodePatrol );
		if( pNode && 
			pNode->IsDefaultNode() &&
			pNode->HasCmd() )
		{
			g_pCmdMgr->QueueCommand( pNode->GetCmd(), m_pAI->m_hObject, m_pAI->m_hObject );
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalScan::ReplanRequired
//
//	PURPOSE:	Return true if AI needs to make a new plan.
//
// ----------------------------------------------------------------------- //

bool CAIGoalScan::ReplanRequired()
{
	// Replan to immediately scan to the default position
	// when senses are turned off.

	if( !m_pAI->GetAIBlackBoard()->GetBBSensesOn() )
	{
		CAIWMFact factQuery;
		factQuery.SetFactType(kFact_Node);
		factQuery.SetNodeType(m_pGoalRecord->eNodeType);
		CAIWMFact* pFact = m_pAI->GetAIWorkingMemory()->FindWMFact( factQuery );
		if( pFact && ( m_hNodePatrol != pFact->GetTargetObject() ) )
		{
			return true;
		}
	}

	// No replanning required.

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalScan::SelectTargetPatrolNode
//
//	PURPOSE:	Select the destination patrol node.
//
// ----------------------------------------------------------------------- //

void CAIGoalScan::AssignTargetPatrolNode( CAIWMFact* pFact )
{
	// Sanity check.

	if( !pFact )
	{
		return;
	}

	// Come to rest immediately when senses are turned off.

	if( !m_pAI->GetAIBlackBoard()->GetBBSensesOn() )
	{
		pFact->SetTargetObject( GetNextPatrolNode(), 1.f );
		return;
	}

	// Default behavior.

	super::AssignTargetPatrolNode( pFact );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalScan::GetNextPatrolNode
//
//	PURPOSE:	Get the next patrol node in the patrol route.
//
// ----------------------------------------------------------------------- //

HOBJECT CAIGoalScan::GetNextPatrolNode()
{
	//
	// Pause at node for specified amount of time.
	//

	AINodeScanner* pNode;
	if( ( m_pAI->GetAIBlackBoard()->GetBBSensesOn() ) &&
		( m_hNodePatrolLast == m_hNodePatrol ) )
	{
		pNode = (AINodeScanner*)g_pLTServer->HandleToObject( m_hNodePatrol );
		if( pNode->GetPauseTime() > 0.f )
		{
			double fArrivalTime = m_pAI->GetAIBlackBoard()->GetBBDestStatusChangeTime();
			if( fArrivalTime + pNode->GetPauseTime() > g_pLTServer->GetTime() )
			{
				return m_hNodePatrol;
			}
		}
	}

	//
	// Scan all nodes when senses are on.
	//

	if( m_pAI->GetAIBlackBoard()->GetBBSensesOn() )
	{
		return super::GetNextPatrolNode();
	}

	//
	// Come to rest at the default node when senses are off.
	//

	pNode = (AINodeScanner*)g_pLTServer->HandleToObject( m_hNodePatrol );
	if( !pNode )
	{
		CAIWMFact factQuery;
		factQuery.SetFactType(kFact_Node);
		factQuery.SetNodeType(m_pGoalRecord->eNodeType);
		CAIWMFact* pFact = m_pAI->GetAIWorkingMemory()->FindWMFact( factQuery );
		if( pFact )
		{
			pNode = (AINodeScanner*)g_pLTServer->HandleToObject( pFact->GetTargetObject() );
		}
	}

	if( !pNode )
	{
		return NULL;
	}

	// Look ahead for default node.

	while( pNode && !pNode->IsDefaultNode() )
	{
		pNode = (AINodeScanner*)pNode->GetNext();
		if( pNode && pNode->m_hObject == m_hNodePatrol )
		{
			break;
		}
	}

	// Found default.

	if( pNode && pNode->IsDefaultNode() )
	{
		return pNode->m_hObject;
	}

	// Look behind for default node.

	pNode = (AINodeScanner*)g_pLTServer->HandleToObject( m_hNodePatrol );
	while( pNode && !pNode->IsDefaultNode() )
	{
		pNode = (AINodeScanner*)pNode->GetPrev();
		if( pNode && pNode->m_hObject == m_hNodePatrol )
		{
			break;
		}
	}

	// Found default.

	if( pNode && pNode->IsDefaultNode() )
	{
		return pNode->m_hObject;
	}

	// No default found.

	return m_hNodePatrol;
}

