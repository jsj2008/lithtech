// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalFlyAway.cpp
//
// PURPOSE : AIGoalFlyAway class implementation
//
// CREATED : 02/04/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIGoalFlyAway.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Goal, CAIGoalFlyAway, kGoal_FlyAway );


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalFlyAway::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAIGoalFlyAway::CAIGoalFlyAway()
{
}

CAIGoalFlyAway::~CAIGoalFlyAway()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalFlyAway::ActivateGoal
//
//	PURPOSE:	Activate the goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalFlyAway::ActivateGoal()
{
	// Default behavior.

	super::ActivateGoal();

	// Allow AI to do limited rotation once flying.

	m_pAI->SetMoveToFloor( false );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalFlyAway::DeactivateGoal
//
//	PURPOSE:	Deactivate the goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalFlyAway::DeactivateGoal()
{
	super::DeactivateGoal();

	// Remove myself after flying away!

	g_pCmdMgr->QueueMessage( m_pAI, m_pAI, "REMOVE" );
}

