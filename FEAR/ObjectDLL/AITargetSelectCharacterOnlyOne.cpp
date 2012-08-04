// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionTargetCharacterOnlyOne.cpp
//
// PURPOSE : AITargetSelectCharacterOnlyOne class definition
//
// CREATED : 5/19/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AITargetSelectCharacterOnlyOne.h"
#include "AI.h"
#include "AITarget.h"
#include "AIBlackBoard.h"
#include "AIWorkingMemory.h"
#include "PlayerObj.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( TargetSelect, CAITargetSelectCharacterOnlyOne, kTargetSelect_CharacterOnlyOne );


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAITargetSelectCharacterOnlyOne::ValidateContextPreconditions
//
//	PURPOSE:	Return true if real-time preconditions are valid.
//
// ----------------------------------------------------------------------- //

bool CAITargetSelectCharacterOnlyOne::ValidatePreconditions( CAI* pAI )
{
	// Intentionally do NOT call super::ValidateContextPreconditions.
	// This selector is only valid if AI is aware of exactly one possible character target.

	// Sanity check.

	if( !pAI )
	{
		return false;
	}

	// Fail if AI was not previously aware of a character target.

	if( !(pAI->GetAIBlackBoard()->GetBBTargetedTypeMask() & kTarget_Character) )
	{
		return false;
	}

	// Fail if AI is aware of more or less than one possible target.

	int iMatches = 0;

	AIWORKING_MEMORY_FACT_LIST::const_iterator itFact;
	const AIWORKING_MEMORY_FACT_LIST* pFactList = pAI->GetAIWorkingMemory()->GetFactList();
	for( itFact = pFactList->begin(); itFact != pFactList->end(); ++itFact )
	{
		// Ignore deleted facts.

		CAIWMFact* pFact = *itFact;
		if( pFact->IsDeleted() )
		{
			continue;
		}

		// Ignore facts which are not about characters.

		if ( pFact->GetFactType() != kFact_Character )
		{
			continue;
		}

		// Ignore dead AI.

		if( IsDeadAI( pFact->GetTargetObject() ) )
		{
			continue;
		}

		++iMatches;

		if ( iMatches > 1 )
		{
			break;
		}
	}

	if ( iMatches != 1 )
	{
		return false;
	}

	// Preconditions are met.

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAITargetSelectCharacterOnlyOne::Validate
//
//	PURPOSE:	Returns true if AI should keep targeting the same target.
//
// ----------------------------------------------------------------------- //

bool CAITargetSelectCharacterOnlyOne::Validate( CAI* pAI )
{
	// Intentionally do NOT call super::Validate.
	// Selector stays valid as long as AI is aware of exactly one possible target.
	// AI does not care if target goes out of view.

	// Target is dead.

	if( IsDeadCharacter( pAI->GetAIBlackBoard()->GetBBTargetObject() ) )
	{
		return false;
	}

	// Selector is no longer valid if AI is aware of more or less than one possible target.

	CAIWMFact factQuery;
	factQuery.SetFactType( kFact_Character );
	if( 1 != pAI->GetAIWorkingMemory()->CountMatches( factQuery ) )
	{
		return false;
	}

	// Selector is no longer valid if targeting a player who has
	// started using a turret.

	if( IsPlayer( pAI->GetAIBlackBoard()->GetBBTargetObject() ) )
	{
		CPlayerObj* pPlayer = (CPlayerObj*)g_pLTServer->HandleToObject( pAI->GetAIBlackBoard()->GetBBTargetObject() );
		if( pPlayer && pPlayer->GetTurret() )
		{
			return false;
		}
	}

	// Target is still valid.

	return true;
}
