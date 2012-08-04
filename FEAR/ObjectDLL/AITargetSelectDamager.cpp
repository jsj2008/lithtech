// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionTargetDamager.cpp
//
// PURPOSE : AITargetSelectDamager class definition
//
// CREATED : 7/15/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AITargetSelectDamager.h"
#include "AI.h"
#include "AIBlackBoard.h"
#include "AIWorkingMemory.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( TargetSelect, CAITargetSelectDamager, kTargetSelect_Damager );


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAITargetSelectDamager::ValidateContextPreconditions
//
//	PURPOSE:	Return true if real-time preconditions are valid.
//
// ----------------------------------------------------------------------- //

bool CAITargetSelectDamager::ValidatePreconditions( CAI* pAI )
{
	// Intentionally do NOT call super::ValidateContextPreconditions.
	// Selector is only valid if we have been damaged by someone recently
	// who is not our current target.

	// Sanity check.

	if( !pAI )
	{
		return false;
	}

	// Do not target damager if senses are turned off.

	if( !pAI->GetAIBlackBoard()->GetBBSensesOn() )
	{
		return false;
	}

	// AI has not taken any damage.

	CAIWMFact factQuery;
	factQuery.SetFactType(kFact_Damage);
	CAIWMFact* pFact = pAI->GetAIWorkingMemory()->FindWMFact( factQuery );
	if( !pFact || !DidDamage(pAI, pFact))
	{
		return false;
	}

	// Damage is not recent.

	if( pFact->GetUpdateTime() < g_pLTServer->GetTime() - 1.f )
	{
		return false;
	}

	// AI is already targeting the damager.

	if( pAI->HasTarget( kTarget_Character | kTarget_Object ) )
	{
		HOBJECT hDamager = pFact->GetTargetObject();
		if( hDamager == pAI->GetAIBlackBoard()->GetBBTargetObject() )
		{
			return false;
		}
	}

	// Preconditions are met.

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAITargetSelectDamager::Activate
//
//	PURPOSE:	Activate selector.
//
// ----------------------------------------------------------------------- //

void CAITargetSelectDamager::Activate( CAI* pAI )
{
	// Intentionally do NOT call super::Activate.
	// Target the source of recent damage.
	//
	// Be sure to call down to the base class however to insure that any 
	// awareness modifications are applied.
	CAITargetSelectAbstract::Activate( pAI );

	// Sanity check.

	if( !pAI )
	{
		return;
	}

	// AI has not taken any damage.

	CAIWMFact factQuery;
	factQuery.SetFactType(kFact_Damage);
	CAIWMFact* pFact = pAI->GetAIWorkingMemory()->FindWMFact( factQuery );
	if( !pFact || !DidDamage(pAI, pFact))
	{
		return;
	}
	HOBJECT hTarget = pFact->GetTargetObject();

	// Damager may be a character, or something else (e.g. Turret).

	ENUM_AIWMFACT_TYPE eFactType = IsCharacter( hTarget ) ? kFact_Character : kFact_Object;

	// Find or create a working memory fact for the damager.

	factQuery.SetFactType( eFactType );
	factQuery.SetTargetObject( hTarget );
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

	// Target the object causing the damage.

	if( eFactType == kFact_Character )
	{
		TargetCharacter( pAI, pFact );
	}
	else {
		TargetObject( pAI, pFact );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAITargetSelectDamager::Validate
//
//	PURPOSE:	Returns true if AI should keep targeting the same target.
//
// ----------------------------------------------------------------------- //

bool CAITargetSelectDamager::Validate( CAI* pAI )
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

	// Do not check visibility for a few seconds.
	// This gives AI time to turn around, when shot from behind.
	// Otherwise, AI may jitter wildly, oscillating between targets.

	if( pAI->GetAIBlackBoard()->GetBBTargetChangeTime() < g_pLTServer->GetTime() - 3.f )
	{
		// Target is no longer in view.

		if( !pAI->GetAIBlackBoard()->GetBBTargetVisibleFromWeapon() )
		{
			return false;
		}
	}

	// Target is still valid.

	return true;
}
