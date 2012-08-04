// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalGuard.cpp
//
// PURPOSE : AIGoalGuard class implementation
//
// CREATED : 4/07/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIGoalGuard.h"
#include "AI.h"
#include "AIDB.h"
#include "AINode.h"
#include "AINodeGuard.h"
#include "AIWorkingMemory.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Goal, CAIGoalGuard, kGoal_Guard );


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalGuard::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAIGoalGuard::CAIGoalGuard()
{
	m_hNodeGuard = NULL;
}

CAIGoalGuard::~CAIGoalGuard()
{
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIGoalGuard::Save/Load
//              
//	PURPOSE:	Handle saving and restoring the CAIGoalGuard
//              
//----------------------------------------------------------------------------

void CAIGoalGuard::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);

	SAVE_HOBJECT(m_hNodeGuard);
}

void CAIGoalGuard::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);

	LOAD_HOBJECT(m_hNodeGuard);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalGuard::CalculateGoalRelevance
//
//	PURPOSE:	Calculate the current goal relevance.
//
// ----------------------------------------------------------------------- //

void CAIGoalGuard::CalculateGoalRelevance()
{
	m_fGoalRelevance = 0.f;

	// Goal is relevant if AI owns a guard node, and is outside of its radius.

	CAIWMFact factQuery;
	factQuery.SetFactType(kFact_Node);
	factQuery.SetNodeType(kNode_Guard);
	CAIWMFact* pFact = m_pAI->GetAIWorkingMemory()->FindWMFact( factQuery );
	if( pFact )
	{
		if (!IsKindOf(pFact->GetTargetObject(), "AINodeGuard"))
		{
			AIASSERT(0, m_pAI->GetHOBJECT(), "CAIGoalGuard::CalculateGoalRelevance : cast to incorrect type.");
			return;
		}

		// Do not return to a node with a zero radius.

		AINodeGuard* pNodeGuard = (AINodeGuard*)g_pLTServer->HandleToObject( pFact->GetTargetObject() );
		if( pNodeGuard && 
			( pNodeGuard->GetNodeOwner() == m_pAI->m_hObject ) &&
			( pNodeGuard->GetRadiusSqr() > 0.f ) &&
			( m_pAI->GetPosition().DistSqr( pNodeGuard->GetPos() ) > pNodeGuard->GetRadiusSqr() ) )
		{
			m_fGoalRelevance = m_pGoalRecord->fIntrinsicRelevance;
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalGuard::ReplanRequired
//
//	PURPOSE:	Return true if AI needs to make a new plan.
//
// ----------------------------------------------------------------------- //

bool CAIGoalGuard::ReplanRequired()
{
	// Replan is required if the guard node in memory does not match 
	// the node we are walking towards.

	CAIWMFact factQuery;
	factQuery.SetFactType(kFact_Node);
	factQuery.SetNodeType(kNode_Guard);
	CAIWMFact* pFact = m_pAI->GetAIWorkingMemory()->FindWMFact( factQuery );
	if( pFact )
	{
		return pFact->GetTargetObject() != m_hNodeGuard;
	}

	// No guard node in memory!

	return true; 
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalGuard::SetWSSatisfaction
//
//	PURPOSE:	Set the WorldState satisfaction conditions.
//
// ----------------------------------------------------------------------- //

void CAIGoalGuard::SetWSSatisfaction( CAIWorldState& WorldState )
{
	CAIWMFact factQuery;
	factQuery.SetFactType(kFact_Node);
	factQuery.SetNodeType(kNode_Guard);
	CAIWMFact* pFact = m_pAI->GetAIWorkingMemory()->FindWMFact( factQuery );
	if( pFact )
	{
		WorldState.SetWSProp( kWSK_AtNode, m_pAI->m_hObject, kWST_HOBJECT, pFact->GetTargetObject() );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalGuard::ActivateGoal
//
//	PURPOSE:	Activate the goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalGuard::ActivateGoal()
{
	super::ActivateGoal();

	CAIWMFact factQuery;
	factQuery.SetFactType(kFact_Node);
	factQuery.SetNodeType(kNode_Guard);
	CAIWMFact* pFact = m_pAI->GetAIWorkingMemory()->FindWMFact( factQuery );
	if( pFact )
	{
		m_hNodeGuard = pFact->GetTargetObject();
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalGuard::DeactivateGoal
//
//	PURPOSE:	Deactivate the goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalGuard::DeactivateGoal()
{
	m_hNodeGuard = NULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalGuard::IsWSSatisfied
//
//	PURPOSE:	Return true if the world state satisfies the goal.
//
// ----------------------------------------------------------------------- //

bool CAIGoalGuard::IsWSSatisfied( CAIWorldState* pwsWorldState )
{
	// Plan is unsatisfyable if AI has no guard node.

	CAIWMFact factQuery;
	factQuery.SetFactType(kFact_Node);
	factQuery.SetNodeType(kNode_Guard);
	CAIWMFact* pFact = m_pAI->GetAIWorkingMemory()->FindWMFact( factQuery );
	if( !pFact )
	{
		return false;
	}

	// The goal is satisfied when we get within the guard node's radius.

	if (!IsKindOf(pFact->GetTargetObject(), "AINodeGuard"))
	{
		AIASSERT(0, m_pAI->GetHOBJECT(), "CAIGoalGuard::IsWSSatisfied : cast to incorrect type.");
		return false;
	}

	AINodeGuard* pNodeGuard = (AINodeGuard*)g_pLTServer->HandleToObject( pFact->GetTargetObject() );
	if( pNodeGuard &&
		( m_pAI->GetPosition().DistSqr( pNodeGuard->GetPos() ) <= pNodeGuard->GetRadiusSqr() ) )
	{
		return true;
	}

	return false;
}


