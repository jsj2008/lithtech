// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionTargetAllyDisturbance.cpp
//
// PURPOSE : AITargetSelectAllyDisturbance class definition
//
// CREATED : 5/19/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AITargetSelectAllyDisturbance.h"
#include "AI.h"
#include "AITarget.h"
#include "AIBlackBoard.h"
#include "AIWorkingMemory.h"
#include "AIUtils.h"
#include "CharacterDB.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( TargetSelect, CAITargetSelectAllyDisturbance, kTargetSelect_AllyDisturbance );


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAITargetSelectAllyDisturbance::ValidateContextPreconditions
//
//	PURPOSE:	Return true if real-time preconditions are valid.
//
// ----------------------------------------------------------------------- //

bool CAITargetSelectAllyDisturbance::ValidatePreconditions( CAI* pAI )
{
	// Intentionally do NOT call super::ValidateContextPreconditions.
	// This Selector is only valid if AI is reacting to a disturbed ally.

	// Sanity check.

	if( !pAI )
	{
		return false;
	}

	// No disturbance.

	CAIWMFact* pFact = pAI->GetAIWorkingMemory()->FindFactDisturbanceMax();
	if( !pFact )
	{
		return false;
	}

	// Disturbance is not from an AI.

	if( !IsAI( pFact->GetTargetObject() ) )
	{
		return false;
	}

	// AI is not an ally.

	CAI* pOther = (CAI*)g_pLTServer->HandleToObject( pFact->GetTargetObject() );
	EnumCharacterStance eStance = g_pCharacterDB->GetStance( pAI->GetAlignment(), pOther->GetAlignment() );
	if( eStance != kCharStance_Like )
	{
		return false;
	}

	// Ally is not targeting a character or object.

	if( ( pOther->GetAIBlackBoard()->GetBBTargetType() != kTarget_Character ) ||
		( pOther->GetAIBlackBoard()->GetBBTargetType() != kTarget_Object ) )
	{
		return false;
	}

	// Target object no longer exists.

	if( !pOther->GetAIBlackBoard()->GetBBTargetObject() )
	{
		return false;
	}

	// Preconditions are met.

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAITargetSelectAllyDisturbance::Activate
//
//	PURPOSE:	Activate selection.
//
// ----------------------------------------------------------------------- //

void CAITargetSelectAllyDisturbance::Activate( CAI* pAI )
{
	// Intentionally do NOT call super::Activate.
	// This Selector sets the target to the character targeted by an Ally.
	//
	// Be sure to call down to the base class however to insure that any 
	// awareness modifications are applied.
	CAITargetSelectAbstract::Activate( pAI );

	// Sanity check.

	if( !pAI )
	{
		return;
	}

	// No disturbance.

	CAIWMFact* pFact = pAI->GetAIWorkingMemory()->FindFactDisturbanceMax();
	if( !pFact )
	{
		return;
	}

	// Disturbance is not from an AI.

	if( !IsAI( pFact->GetTargetObject() ) )
	{
		return;
	}

	// AI is not an ally.

	CAI* pOther = (CAI*)g_pLTServer->HandleToObject( pFact->GetTargetObject() );
	EnumCharacterStance eStance = g_pCharacterDB->GetStance( pAI->GetAlignment(), pOther->GetAlignment() );
	if( eStance != kCharStance_Like )
	{
		return;
	}

	// Ally is not targeting a character or object.

	if( ( pOther->GetAIBlackBoard()->GetBBTargetType() != kTarget_Character ) ||
		( pOther->GetAIBlackBoard()->GetBBTargetType() != kTarget_Object ) )
	{
		return;
	}

	// Bail if the AI is already targeting this character.

	HOBJECT hTarget = pOther->GetAIBlackBoard()->GetBBTargetObject();
	if( hTarget == pAI->GetAIBlackBoard()->GetBBTargetObject() 
		&& ( kTarget_Character == pAI->GetAIBlackBoard()->GetBBTargetType() || kTarget_Object == pAI->GetAIBlackBoard()->GetBBTargetType() ) )
	{
		return;
	}

	// Damager may be a character, or something else (e.g. Turret).

	ENUM_AIWMFACT_TYPE eFactType = IsCharacter( hTarget ) ? kFact_Character : kFact_Object;

	// Target character based on knowledge stored in Ally's working memory.

	CAIWMFact factQuery;
	factQuery.SetFactType( eFactType );
	factQuery.SetTargetObject( hTarget );
	pFact = pOther->GetAIWorkingMemory()->FindWMFact( factQuery );
	if( !pFact )
	{
		return;
	}

	// Find or create a working memory fact for this character.

	CAIWMFact* pTargetFact = pAI->GetAIWorkingMemory()->FindWMFact( factQuery );
	if( !pTargetFact )
	{
		pTargetFact = pAI->GetAIWorkingMemory()->CreateWMFact( eFactType );
		pTargetFact->SetTargetObject( hTarget, 1.f );

		EnumAIStimulusID eStimulusID;
		EnumAIStimulusType eStimulusType;
		pFact->GetStimulus( &eStimulusType, &eStimulusID );
		pTargetFact->SetStimulus( eStimulusType, eStimulusID, 0.f );
		pTargetFact->SetPos( pFact->GetPos(), 1.f );
		pTargetFact->SetRadius( 0.f, 1.f );
		pTargetFact->SetFactFlags( 0 );
	}

	// Target object that ally is targeting.

	if( eFactType == kFact_Character )
	{
		TargetCharacter( pAI, pTargetFact );
	}
	else {
		TargetObject( pAI, pTargetFact );
	}
}
