// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionLongRecoil.cpp
//
// PURPOSE : AIActionLongRecoil class implementation
//
// CREATED : 11/11/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIActionLongRecoil.h"
#include "AI.h"
#include "AIBlackBoard.h"
#include "AIWorkingMemory.h"
#include "AIUtils.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionLongRecoil, kAct_LongRecoil );


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AIActionLongRecoil::GetActionProbability
//
//	PURPOSE:	Return the probability of taking this action.
//
// ----------------------------------------------------------------------- //

float CAIActionLongRecoil::GetActionProbability( CAI* pAI )
{
	// Sanity check.

	if( !pAI )
	{
		return 0.f;
	}

	float fProbability = super::GetActionProbability( pAI );

	// Use the unmodified probability if the AI is taken by surprise. 

	if( pAI->GetAIBlackBoard()->GetBBAwareness() >= kAware_Alert )
	{
		// Bail if no damage fact.

		CAIWMFact factQuery;
		factQuery.SetFactType(kFact_Damage);
		CAIWMFact* pFact = pAI->GetAIWorkingMemory()->FindWMFact( factQuery );
		if (!pFact || !DidDamage(pAI, pFact))
		{
			return 0.f;
		}

		// The confidence of the fact's position indicates the
		// current stimulation level of the damage.
		// Modify the probability by the current stimulation level.
		// This makes the AI more likely to play a long recoil with successive hits.

		fProbability *= pFact->GetConfidence( CAIWMFact::kFactMask_Position );
	}

	return fProbability;
}

