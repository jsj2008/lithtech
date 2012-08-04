// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionTargetCharacterUrgentSpatial.cpp
//
// PURPOSE : AITargetSelectCharacterUrgentSpatial class definition
//
// CREATED : 5/19/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AITargetSelectCharacterUrgentSpatial.h"
#include "AI.h"
#include "AIWorkingMemory.h"
#include "AIPathMgrNavMesh.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( TargetSelect, CAITargetSelectCharacterUrgentSpatial, kTargetSelect_CharacterUrgent );

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAITargetSelectCharacterUrgentSpatial::ValidateContextPreconditions
//
//	PURPOSE:	Return true if real-time preconditions are valid.
//
// ----------------------------------------------------------------------- //

bool CAITargetSelectCharacterUrgentSpatial::ValidatePreconditions( CAI* pAI )
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
//	ROUTINE:	CAITargetSelectCharacterUrgentSpatial::FindValidTarget
//
//	PURPOSE:	Return a valid target.
//
// ----------------------------------------------------------------------- //

CAIWMFact* CAITargetSelectCharacterUrgentSpatial::FindValidTarget( CAI* pAI )
{
	// Intentionally do not call super::FindValidTarget().
	// We are only interested in characters we can attack or pathfind to.

	// Sanity check.

	if( !pAI )
	{
		return NULL;
	}

	//
	// Store all of the potential targets.
	//

	AIASSERT( m_PotentialTargetList.empty(), pAI->GetHOBJECT(), "CAITargetSelectCharacterUrgentSpatial::FindValidTarget: Check logic.  m_PotentialTargetList should be empty." );
	CollectTargets( pAI, m_PotentialTargetList );

	//
	// Sort the targets by distance
	//

	std::sort( m_PotentialTargetList.begin(), m_PotentialTargetList.end() );

	//
	// Find the first that is either in weapon range or that the AI has a path to.
	//

	for ( size_t iEachTarget = 0; iEachTarget < m_PotentialTargetList.size(); ++iEachTarget )
	{
		// Ignore characters if they are out of range to attack, and no path exists to them.

		LTVector vFactPos = m_PotentialTargetList[iEachTarget].m_pFact->GetPos();

		if( ( !AIWeaponUtils::IsPosInRange( pAI, vFactPos, CHECK_HOLSTER ) ) &&
			( !g_pAIPathMgrNavMesh->HasPath( pAI, pAI->GetCharTypeMask(), vFactPos ) ) )
		{
			continue;
		}

		return  m_PotentialTargetList[iEachTarget].m_pFact;
	}

	m_PotentialTargetList.resize( 0 );

	// No valid target found.

	return NULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAITargetSelectCharacterUrgentSpatial::CollectTargets
//
//	PURPOSE:	Fills the passed in TargetListType with all possible 
//				character targets.
//
// ----------------------------------------------------------------------- //

void CAITargetSelectCharacterUrgentSpatial::CollectTargets( CAI* pAI, TargetListType& rOutTargetList ) const
{
	ENUM_AIWeaponType eWeaponType = pAI->GetAIBlackBoard()->GetBBPrimaryWeaponType();

	const AIWORKING_MEMORY_FACT_LIST* pFactList = pAI->GetAIWorkingMemory()->GetFactList();
	AIWORKING_MEMORY_FACT_LIST::const_iterator itFact = pFactList->begin();
	for( ; itFact != pFactList->end(); ++itFact )
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

		// Target is laying on the ground (which doesn't work well with head/aim
		// tracking or with attacks.

		if ( CAI* pTargetAI = CAI::DynamicCast( pAI->GetAIBlackBoard()->GetBBTargetObject() ) )
		{
			if ( pTargetAI->GetAIBlackBoard()->GetBBScriptedBodyState() == eBodyStateKickable )
			{
				continue;
			}
		}

		// Add the target to the list.

		CharacterFact NewFact;
		NewFact.m_flDistanceSqr = pAI->GetPosition().DistSqr( pFact->GetPos() );
		NewFact.m_pFact = pFact;

		rOutTargetList.push_back( NewFact );
	}
}
