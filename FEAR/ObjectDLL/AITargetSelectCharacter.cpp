// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionTargetCharacter.cpp
//
// PURPOSE : AITargetSelectCharacter class definition
//
// CREATED : 5/19/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AITargetSelectCharacter.h"
#include "AI.h"
#include "AITarget.h" 
#include "AIStimulusMgr.h" 
#include "AISoundMgr.h" 
#include "AIBlackBoard.h"
#include "AINavMesh.h"
#include "AINavMeshLinkPlayer.h"
#include "AIPathMgrNavMesh.h"
#include "AIQuadTree.h"
#include "AIWorkingMemory.h"
#include "AICoordinator.h"
#include "CharacterDB.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( TargetSelect, CAITargetSelectCharacter, kTargetSelect_Character );


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAITargetSelectCharacter::Constructor
//
//	PURPOSE:	Constructor.
//
// ----------------------------------------------------------------------- //

CAITargetSelectCharacter::CAITargetSelectCharacter()
{
	m_bRecordFirstThreat = true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAITargetSelectCharacter::ValidateContextPreconditions
//
//	PURPOSE:	Return true if real-time preconditions are valid.
//
// ----------------------------------------------------------------------- //

bool CAITargetSelectCharacter::ValidatePreconditions( CAI* pAI )
{
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

	// Fail if AI is not aware of any character.

	if( !FindValidTarget( pAI ) )
	{
		return false;
	}

	// Preconditions are met.

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAITargetSelectCharacter::Activate
//
//	PURPOSE:	Activate selector.
//
// ----------------------------------------------------------------------- //

void CAITargetSelectCharacter::Activate( CAI* pAI )
{
	super::Activate( pAI );

	// Sanity check.

	if( !pAI )
	{
		return;
	}

	// Bail if AI is not aware of any characters.

	CAIWMFact* pFact = FindValidTarget( pAI );
	if( !pFact )
	{
		return;
	}

	// If we cannot see anyone, target the nearest character.

	if( pFact->GetConfidence( CAIWMFact::kFactMask_Stimulus ) == 0.f )
	{
		CAIWMFact* pNearest = pAI->GetAIWorkingMemory()->FindFactCharacterNearest( pAI );
		if( pNearest )
		{
			// Only target the nearest if he is attackable.

			ENUM_AIWeaponType eWeaponType = pAI->GetAIBlackBoard()->GetBBPrimaryWeaponType();
			if( AIWeaponUtils::IsPosInRange( pAI, pNearest->GetPos(), CHECK_HOLSTER ) ||
				g_pAIPathMgrNavMesh->HasPath( pAI, pAI->GetCharTypeMask(), pNearest->GetPos() ) )
			{
				pFact = pNearest;		
			}
		}
	}

	// Bail if the AI is already targeting this character.

	HOBJECT hTarget = pFact->GetTargetObject();
	if( hTarget == pAI->GetAIBlackBoard()->GetBBTargetObject() 
		&& kTarget_Character == pAI->GetAIBlackBoard()->GetBBTargetType()  )
	{
		return;
	}

	// Target the character.

	TargetCharacter( pAI, pFact );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAITargetSelectCharacter::FindValidTarget
//
//	PURPOSE:	Return a valid target.
//
// ----------------------------------------------------------------------- //

CAIWMFact* CAITargetSelectCharacter::FindValidTarget( CAI* pAI )
{
	// Sanity check.

	if( !pAI )
	{
		return NULL;
	}
	
	ENUM_AIWeaponType eWeaponType = pAI->GetAIBlackBoard()->GetBBPrimaryWeaponType();

	// Prefer to find a character that's in range to attack, or that the 
	// AI can find a path to.

	CAIWMFact* pFact;
	CAIWMFact* pMaxFact = NULL;
	float fMaxConfidence = 0.f;

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

		// Ignore dead AI.

		if( IsDeadAI( pFact->GetTargetObject() ) )
		{
			continue;
		}

		// Keep track of the best seen so far.

		if( fMaxConfidence <= pFact->GetConfidence( CAIWMFact::kFactMask_Stimulus ) )
		{
			fMaxConfidence = pFact->GetConfidence( CAIWMFact::kFactMask_Stimulus );
			pMaxFact = pFact;
		}

		// Prefer to find characters that are in range to attack, or path exists to them.

		if( ( !AIWeaponUtils::IsPosInRange( pAI, pFact->GetPos(), CHECK_HOLSTER ) ) &&
			( !g_pAIPathMgrNavMesh->HasPath( pAI, pAI->GetCharTypeMask(), pFact->GetPos() ) ) )
		{
			continue;
		}

		// Found a prefered target!

		return pFact;
	}

	// Return the best potential target found.

	return pMaxFact;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAITargetSelectCharacter::TargetCharacter
//
//	PURPOSE:	Target a character, and record targeting data.
//
// ----------------------------------------------------------------------- //

void CAITargetSelectCharacter::TargetCharacter( CAI* pAI, CAIWMFact* pFact )
{
	// Sanity check.

	if( !( pAI && pFact ) )
	{
		return;
	}

	HOBJECT hTarget = pFact->GetTargetObject();

	// Record first threat time.
	// Announce threat.

	EnumAIStimulusID eStimulusID;
	EnumAIStimulusType eStimulusType;
	pFact->GetStimulus( &eStimulusType, &eStimulusID );
	if( m_bRecordFirstThreat && ( pAI->GetAIBlackBoard()->GetBBTargetFirstThreatTime() == 0.f ) )
	{
		pAI->GetAIBlackBoard()->SetBBTargetFirstThreatTime( g_pLTServer->GetTime() );

		// Play a sound when a target is spotted.

		if( eStimulusType == kStim_CharacterVisible )
		{
			// Only speak if target truly is visible (as opposed to cheating via seekenemy).
			// Only speak if we are alone.  Otherwise allow squad to determine AI sounds.

			if( pFact->GetConfidence( CAIWMFact::kFactMask_Stimulus ) == 1.f )
			{
				uint32 cSquadMembers = 0;
				ENUM_AI_SQUAD_ID eSquad = g_pAICoordinator->GetSquadID( pAI->m_hObject );
				if( eSquad != kSquad_Invalid )
				{
					CAISquad* pSquad = g_pAICoordinator->FindSquad( eSquad );
					cSquadMembers = pSquad->GetNumSquadMembers();
				}

				////if( cSquadMembers <= 1 )
				{
					if( pFact->GetPos().DistSqr( pAI->GetPosition() ) > g_pAIDB->GetAIConstantsRecord()->fAlertImmediateThreatInstantSeeDistanceSqr )
					{
						g_pAISoundMgr->RequestAISound( pAI->m_hObject, kAIS_DisturbanceSeenAlarmingFar, kAISndCat_Event, hTarget, 0.2f );
					}
					else {
						g_pAISoundMgr->RequestAISound( pAI->m_hObject, kAIS_DisturbanceSeenAlarming, kAISndCat_Event, hTarget, 0.2f );
					}
				}
			}
		}
	}

	// Record new target on the BlackBoard.

	pAI->GetAIBlackBoard()->SetBBTargetType( kTarget_Character );
	pAI->GetAIBlackBoard()->SetBBTargetStimulusType( eStimulusType );
	pAI->GetAIBlackBoard()->SetBBTargetStimulusID( eStimulusID );
	pAI->GetAIBlackBoard()->SetBBTargetChangeTime( g_pLTServer->GetTime() );
	pAI->GetAIBlackBoard()->SetBBTargetObject( hTarget );

	AIASSERT( IsCharacter( hTarget ), pAI->m_hObject, "CAITargetSelectCharacter::TargetCharacter: factObject is not a character" );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAITargetSelectCharacter::TargetObject
//
//	PURPOSE:	Target a object, and record targeting data.
//
// ----------------------------------------------------------------------- //

void CAITargetSelectCharacter::TargetObject( CAI* pAI, CAIWMFact* pFact )
{
	// Sanity check.

	if( !( pAI && pFact ) )
	{
		return;
	}

	HOBJECT hTarget = pFact->GetTargetObject();

	// Target stimulus.

	EnumAIStimulusID eStimulusID;
	EnumAIStimulusType eStimulusType;
	pFact->GetStimulus( &eStimulusType, &eStimulusID );

	// Record new target on the BlackBoard.

	pAI->GetAIBlackBoard()->SetBBTargetType( kTarget_Object );
	pAI->GetAIBlackBoard()->SetBBTargetStimulusType( eStimulusType );
	pAI->GetAIBlackBoard()->SetBBTargetStimulusID( eStimulusID );
	pAI->GetAIBlackBoard()->SetBBTargetChangeTime( g_pLTServer->GetTime() );
	pAI->GetAIBlackBoard()->SetBBTargetObject( hTarget );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAITargetSelectCharacter::Validate
//
//	PURPOSE:	Returns true if AI should keep targeting the same target.
//
// ----------------------------------------------------------------------- //

bool CAITargetSelectCharacter::Validate( CAI* pAI )
{
	// Sanity check.

	if( !pAI )
	{
		return false;
	}

	// Target is dead.

	if( IsDeadCharacter( pAI->GetAIBlackBoard()->GetBBTargetObject() ) )
	{
		return false;
	}

	// Target is no longer in view.

	if( !pAI->GetAIBlackBoard()->GetBBTargetVisibleFromWeapon() )
	{
		return false;
	}

	// Target is still valid.

	return true;
}
