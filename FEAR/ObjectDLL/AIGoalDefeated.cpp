// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalDefeated.cpp
//
// PURPOSE : 
//
// CREATED : 9/21/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIGoalDefeated.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Goal, CAIGoalDefeated, kGoal_Defeated );

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalDefeated::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAIGoalDefeated::CAIGoalDefeated()
{
	// Require the AI satisfy being in defeated state.

	m_eWorldStateEvent = kWSE_Defeated;

	// Do not allow replanning while this goal is executing.  This causes 
	// visible pops when an AI goes from being in defeated state to being 
	// in it again.

	m_bAllowReplanWhenCurGoal = false;

	// Do not play a blended recoil when the AI fails to go into the 
	// defeated state.  This is already handled through the standard 
	// ReactToDamage goal; if both goals did this, the AI would play a 
	// blended reoil all the time.

	m_bPlayRecoilOnBuildPlanFailure = false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalDefeated::ReplanRequired
//
//	PURPOSE:	Return true if AI needs to make a new plan.
//
// ----------------------------------------------------------------------- //

bool CAIGoalDefeated::ReplanRequired()
{
	return false;
}
