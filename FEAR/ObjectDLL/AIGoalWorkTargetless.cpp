// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalWorkTargetless.cpp
//
// PURPOSE : AIGoalWorkTargetless class implementation
//
// CREATED : 9/07/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIGoalWorkTargetless.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Goal, CAIGoalWorkTargetless, kGoal_WorkTargetless );


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalWorkTargetless::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAIGoalWorkTargetless::CAIGoalWorkTargetless()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalWorkTargetless::CalculateGoalRelevance
//
//	PURPOSE:	Calculate the current goal relevance.
//
// ----------------------------------------------------------------------- //

void CAIGoalWorkTargetless::CalculateGoalRelevance()
{
	// Goal is irrelevant if AI has a target,
	// and AI is not in the middle of an animation.

	bool bInProgress = IsGoalInProgress() && m_pAI->GetAnimationContext()->IsLocked();
	if( m_pAI->HasTarget( kTarget_All ) &&
		!bInProgress )
	{
		m_fGoalRelevance = 0.f;
		return;
	}

	// Default behavior.

	super::CalculateGoalRelevance();
}
