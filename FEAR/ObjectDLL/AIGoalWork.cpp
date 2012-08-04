// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalWork.cpp
//
// PURPOSE : AIGoalWork class implementation
//
// CREATED : 10/10/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

// Includes required for AIGoalWork.h

#include "Stdafx.h"
#include "AIGoalWork.h"
#include "AI.h"
#include "AINodeTypes.h"
#include "AIWorkingMemory.h"


DEFINE_AI_FACTORY_CLASS_SPECIFIC( Goal, CAIGoalWork, kGoal_Work );


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalWork::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAIGoalWork::CAIGoalWork()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalPatrol::CalculateGoalRelevance
//
//	PURPOSE:	Calculate the current goal relevance.
//
// ----------------------------------------------------------------------- //

void CAIGoalWork::CalculateGoalRelevance()
{
	// Don't work if we're targeting a threat, and have seen the threat.

	if( m_pAI->HasTarget( kTarget_Character | kTarget_Object ) &&
		( m_pAI->GetAIBlackBoard()->GetBBTargetLastVisibleTime() > 0.f ) )
	{
		m_fGoalRelevance = 0.f;
		return;
	}

	super::CalculateGoalRelevance();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalWork::FindBestNode
//
//	PURPOSE:	Return the best available node.
//
// ----------------------------------------------------------------------- //

HOBJECT CAIGoalWork::FindBestNode( EnumAINodeType eNodeType )
{
	CAIWMFact* pFactResult;

	// We have a Guard node, so randomly choose a guarded node.

	CAIWMFact factQuery;
	factQuery.SetFactType(kFact_Node);
	factQuery.SetNodeType(kNode_Guard);
	CAIWMFact* pFact = m_pAI->GetAIWorkingMemory()->FindWMFact( factQuery );
	if( pFact && pFact->GetTargetObject() )
	{	
		pFactResult = m_pAI->GetAIWorkingMemory()->FindFactNodeRandom( m_pAI, eNodeType, kNodeStatus_All, m_hNode, NULL );
	}

	// We do NOT have a Guard node, so choose the nearest node.

	else {
		pFactResult = m_pAI->GetAIWorkingMemory()->FindFactNodeMax( m_pAI, eNodeType, m_dwNodeStatus, m_hNode, NULL );
	}

	if( pFactResult )
	{
		return pFactResult->GetTargetObject();
	}
	return NULL;
}

