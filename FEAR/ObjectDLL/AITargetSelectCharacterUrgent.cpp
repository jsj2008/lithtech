// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionTargetCharacterUrgent.cpp
//
// PURPOSE : AITargetSelectCharacterUrgent class definition
//
// CREATED : 5/19/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AITargetSelectCharacterUrgent.h"
#include "AI.h"
#include "AIWorkingMemory.h"
#include "AIPathMgrNavMesh.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( TargetSelect, CAITargetSelectCharacterUrgent, kTargetSelect_CharacterUrgent );


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAITargetSelectCharacterUrgent::ValidateContextPreconditions
//
//	PURPOSE:	Return true if real-time preconditions are valid.
//
// ----------------------------------------------------------------------- //

bool CAITargetSelectCharacterUrgent::ValidatePreconditions( CAI* pAI )
{
	// Intentionally do NOT call super::ValidateContextPreconditions.
	// This Selector is only valid if AI 100% confident in a character target.

	// Sanity check.

	if( !pAI )
	{
		return false;
	}

	// Fail if AI is not 100% confident in a character target.

	if( !FindValidTarget( pAI ) )
	{
		return false;
	}

	// Preconditions are met.

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAITargetSelectCharacterUrgent::FindValidTarget
//
//	PURPOSE:	Return a valid target.
//
// ----------------------------------------------------------------------- //

CAIWMFact* CAITargetSelectCharacterUrgent::FindValidTarget( CAI* pAI )
{
	// Intentionally do not call super::FindValidTarget().
	// We are only interested in characters we can attack or pathfind to.

	// Sanity check.

	if( !pAI )
	{
		return NULL;
	}

	ENUM_AIWeaponType eWeaponType = pAI->GetAIBlackBoard()->GetBBPrimaryWeaponType();

	CAIWMFact* pFact;
	AIWORKING_MEMORY_FACT_LIST::const_iterator itFact;
	const AIWORKING_MEMORY_FACT_LIST* pFactList = pAI->GetAIWorkingMemory()->GetFactList();
	for( itFact = pFactList->begin(); itFact != pFactList->end(); ++itFact )
	{
		// Ignore deleted facts.

		pFact = *itFact;
		if( pFact->IsDeleted() )
		{
			continue;
		}

		// Ignore facts which are not about characters.

		if ( pFact->GetFactType() != kFact_Character )
		{
			continue;
		}

		// Ignore fact if AI is not 100% confident in a character's position,
		// and not discovering the position from a squad communication.
		
		if( ( pFact->GetConfidence( CAIWMFact::kFactMask_Position ) < 1.f ) &&
			( !( pAI->GetAIBlackBoard()->GetBBTargetPosTrackingFlags() & kTargetTrack_Squad ) ) )
		{
			continue;
		}

		// Ignore dead AI.

		if( IsDeadAI( pFact->GetTargetObject() ) )
		{
			continue;
		}

		// Ignore characters if they are out of range to attack, and no path exists to them.

		if( ( !AIWeaponUtils::IsPosInRange( pAI, pFact->GetPos(), CHECK_HOLSTER ) ) &&
			( !g_pAIPathMgrNavMesh->HasPath( pAI, pAI->GetCharTypeMask(), pFact->GetPos() ) ) )
		{
			continue;
		}

		// Found a valid target!

		return pFact;
	}

	// No valid target found.

	return NULL;
}





