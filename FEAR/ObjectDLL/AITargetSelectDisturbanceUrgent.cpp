// ----------------------------------------------------------------------- //
//
// MODULE  : AITargetSelectDisturbanceUrgent.cpp
//
// PURPOSE : AITargetSelectDisturbanceUrgent class definition
//
// CREATED : 5/19/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AITargetSelectDisturbanceUrgent.h"
#include "AI.h"
#include "AITarget.h"
#include "AIBlackBoard.h"
#include "AIWorkingMemory.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( TargetSelect, CAITargetSelectDisturbanceUrgent, kTargetSelect_DisturbanceUrgent );


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAITargetSelectDisturbance::Constructor
//
//	PURPOSE:	Factory Constructor
//
// ----------------------------------------------------------------------- //

CAITargetSelectDisturbanceUrgent::CAITargetSelectDisturbanceUrgent()
{
	m_fMinCharacterConfidence = 0.9f;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAITargetSelectDisturbanceUrgent::ValidateContextPreconditions
//
//	PURPOSE:	Return true if real-time preconditions are valid.
//
// ----------------------------------------------------------------------- //

bool CAITargetSelectDisturbanceUrgent::ValidatePreconditions( CAI* pAI )
{
	if( !super::ValidatePreconditions( pAI ) )
	{
		return false;
	}

	// Urgent disturbances distract an AI from targeting a character.
	// Fail if AI is not targeting a character.

	if( pAI->GetAIBlackBoard()->GetBBTargetType() != kTarget_Character )
	{
		return false;
	}

	// Ignore disturbances if AI is at least 90% confident in a character target.

	float fCharConfidence = 0.f;
	CAIWMFact* pFact = pAI->GetAIWorkingMemory()->FindFactCharacterMax( pAI );
	if( pFact )
	{
		fCharConfidence = pFact->GetConfidence( CAIWMFact::kFactMask_Position );
		if( fCharConfidence >= 0.9f )
		{
			return false;
		}
	}

	// Ignore disturbances if AI is more confident in a character.

	pFact = pAI->GetAIWorkingMemory()->FindFactDisturbanceMax();
	if( pFact )
	{
		if( fCharConfidence >= pFact->GetConfidence( CAIWMFact::kFactMask_Position ) )
		{
			return false;
		}
	}

	// Preconditions are met.

	return true;
}
