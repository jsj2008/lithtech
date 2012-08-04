// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalFallBack.cpp
//
// PURPOSE : AIGoalFallBack class implementation
//
// CREATED : 10/10/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

// Includes required for AIGoalFallBack.h

#include "Stdafx.h"
#include "AIGoalFallBack.h"
#include "AI.h"
#include "AITarget.h"
#include "AIWorkingMemory.h"


DEFINE_AI_FACTORY_CLASS_SPECIFIC( Goal, CAIGoalFallBack, kGoal_FallBack );


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalFallBack::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAIGoalFallBack::CAIGoalFallBack()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalFallBack::CalculateGoalRelevance
//
//	PURPOSE:	Calculate the current goal relevance.
//
// ----------------------------------------------------------------------- //

void CAIGoalFallBack::CalculateGoalRelevance()
{
	// Only fall back if we have a desire to do so.

	CAIWMFact queryFact;
	queryFact.SetFactType( kFact_Desire );
	queryFact.SetDesireType( kDesire_FallBack );
	CAIWMFact* pFact = m_pAI->GetAIWorkingMemory()->FindWMFact( queryFact );
	if ( !pFact )
	{
		m_fGoalRelevance = 0.0f;
		return;
	}

	// Default behavior.

	super::CalculateGoalRelevance();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalFallBack::FindBestNode
//
//	PURPOSE:	Return the best available node.
//
// ----------------------------------------------------------------------- //

HOBJECT CAIGoalFallBack::FindBestNode( EnumAINodeType eNodeType )
{
	// Look for a node, applying the threat object (which the baseclass doesn't use)

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
