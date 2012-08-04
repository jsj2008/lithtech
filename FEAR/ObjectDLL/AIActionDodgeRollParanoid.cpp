// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionDodgeRollParanoid.cpp
//
// PURPOSE : AIActionDodgeRollParanoid class implementation
//
// CREATED : 4/28/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIActionDodgeRollParanoid.h"
#include "AI.h"
#include "AIBlackBoard.h"
#include "NodeTrackerContext.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionDodgeRollParanoid, kAct_DodgeRollParanoid );


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionDodgeRollParanoid::ValidateContextPreconditions
//
//	PURPOSE:	Return true if real-time preconditions are valid.
//
// ----------------------------------------------------------------------- //

bool CAIActionDodgeRollParanoid::ValidateContextPreconditions( CAI* pAI, CAIWorldState& wsWorldStateGoal, bool bIsPlanning )
{
	// Do not roll if we have rolled too recently.

	float fRollTime = 3.f;
	if( g_pLTServer->GetTime() - pAI->GetAIBlackBoard()->GetBBPostureChangeTime() < fRollTime )
	{
		return false;
	}

	// Skip the super classes' validate, so that we don't limit the posture change time.

	return CAIActionDodgeShuffle::ValidateContextPreconditions( pAI, wsWorldStateGoal, bIsPlanning );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionDodgeRollParanoid::ActivateAction
//
//	PURPOSE:	Activate action.
//
// ----------------------------------------------------------------------- //

void CAIActionDodgeRollParanoid::ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal )
{
	super::ActivateAction( pAI, wsWorldStateGoal );

	// Torso tracking.

	pAI->GetAIBlackBoard()->SetBBTargetTrackerFlags( kTrackerFlag_None );
	pAI->GetAIBlackBoard()->SetBBFaceTarget( false );
}

