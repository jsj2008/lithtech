// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalUseSmartObject.cpp
//
// PURPOSE : AIGoalUseSmartObject class implementation
//
// CREATED : 2/28/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIGoalUseSmartObject.h"
#include "AI.h"
#include "AIDB.h"
#include "AINode.h"
#include "AIWorkingMemory.h"
#include "AIUtils.h"


DEFINE_AI_FACTORY_CLASS_SPECIFIC( Goal, CAIGoalUseSmartObject, kGoal_UseSmartObject );


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalUseSmartObject::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAIGoalUseSmartObject::CAIGoalUseSmartObject()
{
	m_hNode = NULL;
	m_hNodeBest = NULL;

	m_dwNodeStatus = kNodeStatus_All;
}

CAIGoalUseSmartObject::~CAIGoalUseSmartObject()
{
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIGoalUseSmartObject::Save/Load
//              
//	PURPOSE:	Handle saving and restoring the CAIGoalUseSmartObject
//              
//----------------------------------------------------------------------------

void CAIGoalUseSmartObject::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);

	SAVE_HOBJECT(m_hNode);
	SAVE_HOBJECT(m_hNodeBest);
}

void CAIGoalUseSmartObject::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);

	LOAD_HOBJECT(m_hNode);
	LOAD_HOBJECT(m_hNodeBest);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalUseSmartObject::CalculateGoalRelevance
//
//	PURPOSE:	Calculate the current goal relevance.
//
// ----------------------------------------------------------------------- //

void CAIGoalUseSmartObject::CalculateGoalRelevance()
{
	// Goal is in progress.

	if( IsGoalInProgress() )
	{
		m_fGoalRelevance = m_pGoalRecord->fIntrinsicRelevance;
		return;
	}

	// Goal is relevant if we know of available nodes.

	m_hNodeBest = FindBestNode( m_pGoalRecord->eNodeType );
	if( m_hNodeBest )
	{
		m_fGoalRelevance = m_pGoalRecord->fIntrinsicRelevance;
		return;
	}

	m_fGoalRelevance = 0.f;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalUseSmartObject::IsGoalInProgress
//
//	PURPOSE:	Return true if goal is already in progress.
//
// ----------------------------------------------------------------------- //

bool CAIGoalUseSmartObject::IsGoalInProgress()
{
	// No current node.

	if( !m_hNode )
	{
		return false;
	}

	// Node is disabled.

	AINode* pNode = (AINode*)g_pLTServer->HandleToObject( m_hNode );
	if( pNode && pNode->IsNodeDisabled() )
	{
		return false;
	}

	// Node is owned by a disabled node.

	if( IsAINode( pNode->GetNodeOwner() ) )
	{
		AINode* pNodeOwner = (AINode*)g_pLTServer->HandleToObject( pNode->GetNodeOwner() );
		if( pNodeOwner && pNodeOwner->IsNodeDisabled() )
		{
			return false;
		}
	}

	// Goal is satisfied.

	if( IsWSSatisfied( m_pAI->GetAIWorldState() ) )
	{
		return false;
	}
	
	// Goal is still in progress.

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalUseSmartObject::FindBestNode
//
//	PURPOSE:	Return the best available node.
//
// ----------------------------------------------------------------------- //

HOBJECT CAIGoalUseSmartObject::FindBestNode( EnumAINodeType eNodeType )
{
	CAIWMFact* pFact = m_pAI->GetAIWorkingMemory()->FindFactNodeMax( m_pAI, eNodeType, m_dwNodeStatus, m_hNode, NULL );
	if( pFact )
	{
		return pFact->GetTargetObject();
	}
	return NULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalUseSmartObject::ReplanRequired
//
//	PURPOSE:	Return true if AI needs to make a new plan.
//
// ----------------------------------------------------------------------- //

bool CAIGoalUseSmartObject::ReplanRequired()
{
	// Replan if the destination has changed.

	if(	m_hNode != m_hNodeBest )
	{
		return true;
	}

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalUseSmartObject::ActivateGoal
//
//	PURPOSE:	Activate the goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalUseSmartObject::ActivateGoal()
{
	super::ActivateGoal();

	m_hNode = m_hNodeBest;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalUseSmartObject::DeactivateGoal
//
//	PURPOSE:	Deactivate the goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalUseSmartObject::DeactivateGoal()
{
	super::DeactivateGoal();

	m_hNode = NULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalUseSmartObject::SetWSSatisfaction
//
//	PURPOSE:	Set the WorldState satisfaction conditions.
//
// ----------------------------------------------------------------------- //

void CAIGoalUseSmartObject::SetWSSatisfaction( CAIWorldState& WorldState )
{
	// Set satisfaction properties.

	WorldState.SetWSProp( kWSK_UsingObject, m_pAI->m_hObject, kWST_HOBJECT, m_hNodeBest );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalUseSmartObject::IsWSSatisfied
//
//	PURPOSE:	Return true if the world state satisfies the goal.
//
// ----------------------------------------------------------------------- //

bool CAIGoalUseSmartObject::IsWSSatisfied( CAIWorldState* pwsWorldState )
{
	SAIWORLDSTATE_PROP* pProp = pwsWorldState->GetWSProp( kWSK_UsingObject, m_pAI->m_hObject );
	if( pProp && pProp->hWSValue && ( pProp->hWSValue == m_hNode ) )
	{
		return true;
	}

	return false;
}


