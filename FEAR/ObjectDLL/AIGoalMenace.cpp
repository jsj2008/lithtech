// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalMenace.cpp
//
// PURPOSE : 
//
// CREATED : 12/13/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIGoalMenace.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Goal, CAIGoalMenace, kGoal_Menace );

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalMenace::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAIGoalMenace::CAIGoalMenace()
{
}

CAIGoalMenace::~CAIGoalMenace()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalPatrol::CalculateGoalRelevance
//
//	PURPOSE:	Calculate the current goal relevance.
//
// ----------------------------------------------------------------------- //

void CAIGoalMenace::CalculateGoalRelevance()
{
	// Don't menace if we're targeting a threat, and have seen the threat.

	if( m_pAI->HasTarget( kTarget_Character | kTarget_Object ) )
	{
		m_fGoalRelevance = 0.f;
		return;
	}

	super::CalculateGoalRelevance();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalMenace::FindBestNode
//
//	PURPOSE:	Return the best available node. Look for a Menace task, 
//				to support scripted menacing.  If a match is not found,
//				search for a standard menance node to support typical 
//				behavior.
//
// ----------------------------------------------------------------------- //

HOBJECT CAIGoalMenace::FindBestNode( EnumAINodeType eNodeType )
{
	CAIWMFact queryFact;
	queryFact.SetFactType( kFact_Task );
	queryFact.SetTaskType( kTask_Menace );
	CAIWMFact* pFact = m_pAI->GetAIWorkingMemory()->FindWMFact( queryFact );
	if ( pFact )
	{
		return pFact->GetTargetObject();
	}

	return super::FindBestNode( eNodeType );
}
