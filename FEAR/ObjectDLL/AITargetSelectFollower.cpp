// ----------------------------------------------------------------------- //
//
// MODULE  : AITargetSelectFollower.cpp
//
// PURPOSE : 
//
// CREATED : 4/11/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AITargetSelectFollower.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( TargetSelect, CAITargetSelectFollower, kTargetSelect_Follower );

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAITargetSelectFollower::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAITargetSelectFollower::CAITargetSelectFollower()
{
}

CAITargetSelectFollower::~CAITargetSelectFollower()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAITargetSelectFollower::ValidateContextPreconditions
//
//	PURPOSE:	Return true if real-time preconditions are valid.
//
// ----------------------------------------------------------------------- //

bool CAITargetSelectFollower::ValidatePreconditions( CAI* pAI )
{
	// Sanity check.

	if( !pAI )
	{
		return false;
	}

	// Bail if we have no follow task.

	CAIWMFact factQuery;
	factQuery.SetFactType( kFact_Task );
	factQuery.SetTaskType( kTask_LeadCharacter );
	CAIWMFact* pFact = pAI->GetAIWorkingMemory()->FindWMFact( factQuery );
	if( !pFact )
	{
		return false;
	}

	// Preconditions are met.

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAITargetSelectFollower::Activate
//
//	PURPOSE:	Activate selector.
//
// ----------------------------------------------------------------------- //

void CAITargetSelectFollower::Activate( CAI* pAI )
{
	super::Activate( pAI );

	// Sanity check.

	if( !pAI )
	{
		return;
	}

	// Bail if we have no follow task.

	CAIWMFact factQuery;
	factQuery.SetFactType( kFact_Task );
	factQuery.SetTaskType( kTask_LeadCharacter );
	CAIWMFact* pFact = pAI->GetAIWorkingMemory()->FindWMFact( factQuery );
	if( !pFact )
	{
		return;
	}

	// Record new target on the BlackBoard.

	pAI->GetAIBlackBoard()->SetBBTargetType( kTarget_Follower );
	pAI->GetAIBlackBoard()->SetBBTargetStimulusType( kStim_InvalidType );
	pAI->GetAIBlackBoard()->SetBBTargetStimulusID( kStimID_Invalid );
	pAI->GetAIBlackBoard()->SetBBTargetChangeTime( g_pLTServer->GetTime() );
	pAI->GetAIBlackBoard()->SetBBTargetObject( pFact->GetSourceObject() );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAITargetSelectFollower::Validate
//
//	PURPOSE:	Returns true if AI should keep targeting the same target.
//
// ----------------------------------------------------------------------- //

bool CAITargetSelectFollower::Validate( CAI* pAI )
{
	// Sanity check.

	if( !pAI )
	{
		return false;
	}

	// Target is dead.

	if( IsDeadAI( pAI->GetAIBlackBoard()->GetBBTargetObject() ) )
	{
		return false;
	}

	// Bail if we have no follow task.

	CAIWMFact factQuery;
	factQuery.SetFactType( kFact_Task );
	factQuery.SetTaskType( kTask_LeadCharacter );
	CAIWMFact* pFact = pAI->GetAIWorkingMemory()->FindWMFact( factQuery );
	if( !pFact )
	{
		return false;
	}

	// Target is still valid.

	return true;
}
