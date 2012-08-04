// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalGotoCombat.cpp
//
// PURPOSE : AIGoalGotoCombat class implementation
//
// CREATED : 3/03/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

// Includes required for AIGoalGotoCombat.h

#include "Stdafx.h"
#include "AIGoalAbstract.h"
#include "AIGoalGoto.h"
#include "AIGoalGotoCombat.h"

// Includes required for AIGoalGotoCombat.cpp

#include "AI.h"
#include "AIWorkingMemory.h"
#include "AITarget.h"
#include "AINode.h"
#include "AIGoalButeMgr.h"


DEFINE_AI_FACTORY_CLASS_SPECIFIC( Goal, CAIGoalGotoCombat, kGoal_GotoCombat );


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalGotoCombat::CalculateGoalRelevance

//
//	PURPOSE:	Calculate the current goal relevance.
//
// ----------------------------------------------------------------------- //

void CAIGoalGotoCombat::CalculateGoalRelevance()
{
	// Goal is already active or we are aware of an enemy.

	if( m_hNode || m_pAI->HasTarget( kTarget_Character ) )
	{
		super::CalculateGoalRelevance();
		return;
	}

	// No enemy present.

	m_fGoalRelevance = 0.f;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalGotoCombat::FindBestNode
//
//	PURPOSE:	Return the best available node.
//
// ----------------------------------------------------------------------- //

CAIWMFact* CAIGoalGotoCombat::FindBestNode( EnumAINodeType eNodeType )
{
	if( m_pAI->HasTarget( kTarget_Character ) )
	{
		return m_pAI->GetAIWorkingMemory()->FindFactNodeMax( m_pAI, eNodeType, m_hNode, m_pAI->GetTarget()->GetObject() );
	}

	return NULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalGotoCombat::ActivateGoal
//
//	PURPOSE:	Activate the goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalGotoCombat::ActivateGoal()
{
	super::ActivateGoal();

	// Find an existing memory for the desire to search, or create one.

	CAIWMFact* pFact = m_pAI->GetAIWorkingMemory()->FindWMFact( kFact_Desire, kDesire_Search );
	if( !pFact )
	{
		pFact = m_pAI->GetAIWorkingMemory()->CreateWMFact( kFact_Desire );
	}

	// Set the current desire.

	if( pFact )
	{
		pFact->factDesireType.eDesireType = kDesire_Search;
		pFact->factDesireType.fConfidence = 1.f;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalGotoCombat::IsWSSatisfied
//
//	PURPOSE:	Return true if the world state satisfies the goal.
//
// ----------------------------------------------------------------------- //

bool CAIGoalGotoCombat::IsWSSatisfied( bool bIsPlanning, CAIWorldState* pwsWorldState )
{
	HOBJECT hNode = bIsPlanning ? m_hNodeBest : m_hNode;

	// Get the node type for this goal.

	EnumAINodeType eNodeType = kNode_InvalidType;
	AIGBM_GoalTemplate* pTemplate = g_pAIGoalButeMgr->GetTemplate( m_eGoalType );
	if( pTemplate )
	{
		eNodeType = pTemplate->eNodeType;
	}

	HOBJECT hThreat = NULL;
	if( m_pAI->HasTarget( kTarget_Character ) )
	{
		hThreat = m_pAI->GetTarget()->GetObject();
	}

	// Goal is satisfied if we are at a valid node of the specified type.

	SAIWORLDSTATE_PROP* pProp = pwsWorldState->GetWSProp( kWSK_AtNode, m_pAI->m_hObject );
	if( pProp )
	{
		AINode* pNode = (AINode*)g_pLTServer->HandleToObject( pProp->hWSValue );
		if( pNode && 
			( pNode->GetType() == eNodeType ) &&
			( pNode->GetStatus( m_pAI->GetPosition(), hThreat ) == kStatus_Ok ) )
		{
			return true;
		}
	}

	return false;
}

