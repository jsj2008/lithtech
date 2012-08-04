// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalPatrol.cpp
//
// PURPOSE : AIGoalPatrol class implementation
//
// CREATED : 1/28/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIGoalPatrol.h"
#include "AI.h"
#include "AIDB.h"
#include "AINode.h"
#include "AIWorkingMemory.h"
#include "AIUtils.h"


DEFINE_AI_FACTORY_CLASS_SPECIFIC( Goal, CAIGoalPatrol, kGoal_Patrol );


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalPatrol::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAIGoalPatrol::CAIGoalPatrol()
{
	m_hNodePatrol = NULL;
	m_hNodePatrolLast = NULL;
	bPatrolReverse = false;
}

CAIGoalPatrol::~CAIGoalPatrol()
{
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIGoalPatrol::Save/Load
//              
//	PURPOSE:	Handle saving and restoring the CAIGoalPatrol
//              
//----------------------------------------------------------------------------

void CAIGoalPatrol::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);

	SAVE_HOBJECT(m_hNodePatrol);
	SAVE_HOBJECT(m_hNodePatrolLast);
	SAVE_bool(bPatrolReverse);
}

void CAIGoalPatrol::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);

	LOAD_HOBJECT(m_hNodePatrol);
	LOAD_HOBJECT(m_hNodePatrolLast);
	LOAD_bool(bPatrolReverse);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalPatrol::CalculateGoalRelevance
//
//	PURPOSE:	Calculate the current goal relevance.
//
// ----------------------------------------------------------------------- //

void CAIGoalPatrol::CalculateGoalRelevance()
{
	m_fGoalRelevance = 0.f;

	// Don't patrol if we're attached to something.

	if( m_pAI->GetAIBlackBoard()->GetBBAttachedTo() )
	{
		return;
	}

	// Don't patrol if we're targeting a threat, and have seen the threat.

	if( m_pAI->HasTarget( kTarget_Character | kTarget_Object ) &&
		( m_pAI->GetAIBlackBoard()->GetBBTargetLastVisibleTime() > 0.f ) &&
		( m_pAI->GetAIBlackBoard()->GetBBSensesOn() ) )
	{
		return;
	}


	CAIWMFact factQuery;
	factQuery.SetFactType(kFact_Node);
	factQuery.SetNodeType(m_pGoalRecord->eNodeType);
	CAIWMFact* pFact = m_pAI->GetAIWorkingMemory()->FindWMFact( factQuery );
	if( pFact )
	{
		AssignTargetPatrolNode( pFact );

		// The goal is relevant if there is an owned patrol node to go to.

		AINode* pNode = (AINode*)g_pLTServer->HandleToObject( pFact->GetTargetObject() );
		if( pNode && ( pNode->GetNodeOwner() == m_pAI->m_hObject ) )
		{
			m_fGoalRelevance = pFact->GetConfidence( CAIWMFact::kFactMask_Position ) * m_pGoalRecord->fIntrinsicRelevance;
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalPatrol::SetWSSatisfaction
//
//	PURPOSE:	Set the WorldState satisfaction conditions.
//
// ----------------------------------------------------------------------- //

void CAIGoalPatrol::SetWSSatisfaction( CAIWorldState& WorldState )
{
	CAIWMFact factQuery;
	factQuery.SetFactType(kFact_Node);
	factQuery.SetNodeType(m_pGoalRecord->eNodeType);
	CAIWMFact* pFact = m_pAI->GetAIWorkingMemory()->FindWMFact( factQuery );
	if( pFact )
	{
		WorldState.SetWSProp( kWSK_UsingObject, m_pAI->m_hObject, kWST_HOBJECT, pFact->GetTargetObject() );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalPatrol::ActivateGoal
//
//	PURPOSE:	Activate the goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalPatrol::ActivateGoal()
{
	super::ActivateGoal();

	CAIWMFact factQuery;
	factQuery.SetFactType(kFact_Node);
	factQuery.SetNodeType(m_pGoalRecord->eNodeType);
	CAIWMFact* pFact = m_pAI->GetAIWorkingMemory()->FindWMFact( factQuery );
	if( pFact )
	{
		m_hNodePatrol = pFact->GetTargetObject();
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalPatrol::SelectTargetPatrolNode
//
//	PURPOSE:	Select the destination patrol node.
//
// ----------------------------------------------------------------------- //

void CAIGoalPatrol::AssignTargetPatrolNode( CAIWMFact* pFact )
{
	// Sanity check.

	if( !pFact )
	{
		return;
	}

	// Find the next node in the patrol path if the AI is at
	// the destination patrol node.

	SAIWORLDSTATE_PROP* pProp = m_pAI->GetAIWorldState()->GetWSProp( kWSK_UsingObject, m_pAI->m_hObject );
	if( ( pProp && pProp->hWSValue ) && 
	   ( pProp->hWSValue == m_hNodePatrol ) )
	{
		m_hNodePatrolLast = m_hNodePatrol;
		pFact->SetTargetObject( GetNextPatrolNode(), 1.f );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalPatrol::GetNextPatrolNode
//
//	PURPOSE:	Get the next patrol node in the patrol route.
//
// ----------------------------------------------------------------------- //

HOBJECT CAIGoalPatrol::GetNextPatrolNode()
{
	AINodePatrol* pNodePatrol = (AINodePatrol*)g_pLTServer->HandleToObject( m_hNodePatrol );
	if( !pNodePatrol )
	{
		return NULL;
	}

	AINodePatrol* pNext = pNodePatrol->GetNext();
	AINodePatrol* pPrev = pNodePatrol->GetPrev();

	// Patrol forward.

	if( !bPatrolReverse )
	{
		if( pNext && ( pNext->m_hObject != m_hNodePatrolLast ) )
		{
			return pNext->m_hObject;
		}
		else if( pPrev )
		{
			bPatrolReverse = true;
			return pPrev->m_hObject;
		}
	}

	// Patrol reverse.

	else {
		if( pPrev && ( pPrev->m_hObject != m_hNodePatrolLast ) )
		{
			return pPrev->m_hObject;
		}
		else if( pNext )
		{
			bPatrolReverse = false;
			return pNext->m_hObject;
		}
	}

	return NULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalPatrol::IsWSSatisfied
//
//	PURPOSE:	Return true if the world state satisfies the goal.
//
// ----------------------------------------------------------------------- //

bool CAIGoalPatrol::IsWSSatisfied( CAIWorldState* pwsWorldState )
{
	CAIWMFact factQuery;
	factQuery.SetFactType(kFact_Node);
	factQuery.SetNodeType(m_pGoalRecord->eNodeType);
	CAIWMFact* pFact = m_pAI->GetAIWorkingMemory()->FindWMFact( factQuery );	
	if( !pFact )
	{
		return false;
	}

	SAIWORLDSTATE_PROP* pProp = pwsWorldState->GetWSProp( kWSK_UsingObject, m_pAI->m_hObject );
	if( pProp && ( pProp->hWSValue == pFact->GetTargetObject() ) )
	{
		return true;
	}

	return false;
}


