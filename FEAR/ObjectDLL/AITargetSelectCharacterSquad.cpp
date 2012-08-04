// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionTargetCharacterSquad.cpp
//
// PURPOSE : AITargetSelectCharacterSquad class definition
//
// CREATED : 8/24/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AITargetSelectCharacterSquad.h"
#include "AI.h"
#include "AITarget.h" 
#include "AICoordinator.h" 
#include "AIBlackBoard.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( TargetSelect, CAITargetSelectCharacterSquad, kTargetSelect_CharacterSquad );


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAITargetSelectCharacterSquad::ValidateContextPreconditions
//
//	PURPOSE:	Return true if real-time preconditions are valid.
//
// ----------------------------------------------------------------------- //

bool CAITargetSelectCharacterSquad::ValidatePreconditions( CAI* pAI )
{
	// Sanity check.

	if( !pAI )
	{
		return false;
	}
	
	// AI is already targeting a character.

	if( pAI->HasTarget( kTarget_Character ) )
	{
		return false;
	}

	// AI is not in a squad.

	ENUM_AI_SQUAD_ID eSquad = g_pAICoordinator->GetSquadID( pAI->m_hObject );
	CAISquad* pSquad = g_pAICoordinator->FindSquad( eSquad );
	if( !pSquad )
	{
		return false;
	}

	// Find a squad member targeting a character.

	CAI* pMember = NULL;
	bool bTargetingCharacter = false;
	uint32 cMembers = pSquad->GetNumSquadMembers();
	LTObjRef* pMembers = pSquad->GetSquadMembers();
	if( pMembers )
	{
		for( uint32 iMember=0; iMember < cMembers; ++iMember )
		{
			pMember = (CAI*)g_pLTServer->HandleToObject( pMembers[iMember] );
			if( pMember && pMember->HasTarget( kTarget_Character ) )
			{
				bTargetingCharacter = true;
				break;
			}
		}
	}

	// Preconditions are met if the squad is targeting someone.

	return bTargetingCharacter;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAITargetSelectCharacter::Activate
//
//	PURPOSE:	Activate selector.
//
// ----------------------------------------------------------------------- //

void CAITargetSelectCharacterSquad::Activate( CAI* pAI )
{
	super::Activate( pAI );

	// Sanity check.

	if( !pAI )
	{
		return;
	}

	// Bail if AI is not in a squad.

	ENUM_AI_SQUAD_ID eSquad = g_pAICoordinator->GetSquadID( pAI->m_hObject );
	CAISquad* pSquad = g_pAICoordinator->FindSquad( eSquad );
	if( !pSquad )
	{
		return;
	}

	// Find a squad member targeting a character.

	CAI* pMember = NULL;
	bool bTargetingCharacter = false;
	uint32 cMembers = pSquad->GetNumSquadMembers();
	LTObjRef* pMembers = pSquad->GetSquadMembers();
	if( pMembers )
	{
		for( uint32 iMember=0; iMember < cMembers; ++iMember )
		{
			pMember = (CAI*)g_pLTServer->HandleToObject( pMembers[iMember] );
			if( pMember && pMember->HasTarget( kTarget_Character ) )
			{
				break;
			}
		}
	}

	// Bail if we failed to find a squad member.

	if( !pMember )
	{
		return;
	}

	// Bail if squad member is not targeting a character.

	if( !pMember->HasTarget( kTarget_Character ) )
	{
		return;
	}
	HOBJECT hTarget = pMember->GetAIBlackBoard()->GetBBTargetObject();

	// Bail if squad member has no memory of the character.

	CAIWMFact factQuery;
	factQuery.SetFactType( kFact_Character );
	factQuery.SetTargetObject( hTarget );
	CAIWMFact* pFact = pMember->GetAIWorkingMemory()->FindWMFact( factQuery );
	if( !pFact )
	{
		return;
	}

	// Create a memory for this character.

	CAIWMFact* pTargetFact;
	pTargetFact = pAI->GetAIWorkingMemory()->CreateWMFact( kFact_Character );
	pTargetFact->SetTargetObject( hTarget, 1.f );

	EnumAIStimulusID eStimulusID;
	EnumAIStimulusType eStimulusType;
	pFact->GetStimulus( &eStimulusType, &eStimulusID );
	pTargetFact->SetStimulus( eStimulusType, eStimulusID, 0.f );
	pTargetFact->SetPos( pFact->GetPos(), 1.f );
	pTargetFact->SetRadius( 0.f, 1.f );

	// Target the character.

	TargetCharacter( pAI, pTargetFact );
}

