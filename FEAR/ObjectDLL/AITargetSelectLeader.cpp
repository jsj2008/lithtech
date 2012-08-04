// ----------------------------------------------------------------------- //
//
// MODULE  : AITargetSelectLeader.cpp
//
// PURPOSE : AITargetSelectLeader class definition
//
// CREATED : 07/20/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AITargetSelectLeader.h"
#include "AI.h"
#include "AITarget.h" 
#include "AIBlackBoard.h"
#include "AIWorkingMemory.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( TargetSelect, CAITargetSelectLeader, kTargetSelect_Leader );


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAITargetSelectLeader::ValidateContextPreconditions
//
//	PURPOSE:	Return true if real-time preconditions are valid.
//
// ----------------------------------------------------------------------- //

bool CAITargetSelectLeader::ValidatePreconditions( CAI* pAI )
{
	// Sanity check.

	if( !pAI )
	{
		return false;
	}

	// Bail if we have no follow task.

	CAIWMFact factQuery;
	factQuery.SetFactType( kFact_Task );
	factQuery.SetTaskType( kTask_Follow );
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
//	ROUTINE:	CAITargetSelectLeader::Activate
//
//	PURPOSE:	Activate selector.
//
// ----------------------------------------------------------------------- //

void CAITargetSelectLeader::Activate( CAI* pAI )
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
	factQuery.SetTaskType( kTask_Follow );
	CAIWMFact* pFact = pAI->GetAIWorkingMemory()->FindWMFact( factQuery );
	if( !pFact )
	{
		return;
	}

	// Record new target on the BlackBoard.

	pAI->GetAIBlackBoard()->SetBBTargetType( kTarget_Leader );
	pAI->GetAIBlackBoard()->SetBBTargetStimulusType( kStim_InvalidType );
	pAI->GetAIBlackBoard()->SetBBTargetStimulusID( kStimID_Invalid );
	pAI->GetAIBlackBoard()->SetBBTargetChangeTime( g_pLTServer->GetTime() );
	pAI->GetAIBlackBoard()->SetBBTargetObject( pFact->GetTargetObject() );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAITargetSelectLeader::Validate
//
//	PURPOSE:	Returns true if AI should keep targeting the same target.
//
// ----------------------------------------------------------------------- //

bool CAITargetSelectLeader::Validate( CAI* pAI )
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
	factQuery.SetTaskType( kTask_Follow );
	CAIWMFact* pFact = pAI->GetAIWorkingMemory()->FindWMFact( factQuery );
	if( !pFact )
	{
		return false;
	}

	// Target is still valid.

	return true;
}
