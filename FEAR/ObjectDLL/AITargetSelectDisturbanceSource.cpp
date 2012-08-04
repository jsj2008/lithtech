// ----------------------------------------------------------------------- //
//
// MODULE  : AITargetSelectDisturbanceSource.cpp
//
// PURPOSE : AITargetSelectDisturbanceSource class definition
//
// CREATED : 9/20/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AITargetSelectDisturbanceSource.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( TargetSelect, CAITargetSelectDisturbanceSource, kTargetSelect_DisturbanceSource );


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAITargetSelectDisturbanceSource::ValidateContextPreconditions
//
//	PURPOSE:	Return true if real-time preconditions are valid.
//
// ----------------------------------------------------------------------- //

bool CAITargetSelectDisturbanceSource::ValidatePreconditions( CAI* pAI )
{
	// Intentionally do NOT call super::ValidateContextPreconditions.
	// This Selector is only valid if AI is reacting to a disturbance outside his guard area.

	// Sanity check.

	if( !pAI )
	{
		return false;
	}

	// Fail if AI was previously aware of a character or object target.

	if( pAI->GetAIBlackBoard()->GetBBTargetedTypeMask() & ( kTarget_Character | kTarget_Object ) )
	{
		return false;
	}

	// No disturbance.

	CAIWMFact* pFact = pAI->GetAIWorkingMemory()->FindFactDisturbanceMax();
	if( !pFact )
	{
		return false;
	}

	// Disturbance is not recent.

	if( g_pLTServer->GetTime() - pFact->GetUpdateTime() > 1.f )
	{
		return false;
	}

	// Preconditions are met.

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAITargetSelectDisturbanceSource::Activate
//
//	PURPOSE:	Activate selector.
//
// ----------------------------------------------------------------------- //

void CAITargetSelectDisturbanceSource::Activate( CAI* pAI )
{
	// Intentionally do NOT call super::Activate.
	// This Selector is only valid if AI is reacting to a disturbance outside his guard area.
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

	// Target the disturbance's source.

	LTVector vPos;
	HOBJECT hTarget = pFact->GetTargetObject();
	g_pLTServer->GetObjectPos( hTarget, &vPos );

	// Bail if the AI is already targeting this object.

	if( hTarget == pAI->GetAIBlackBoard()->GetBBTargetObject() )
	{
		return;
	}

	// Source may be a character, or something else.

	ENUM_AIWMFACT_TYPE eFactType = IsCharacter( hTarget ) ? kFact_Character : kFact_Object;

	// Find or create a working memory fact for this character.

	CAIWMFact factQuery;
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
		pTargetFact->SetPos( vPos, 1.f );
		pTargetFact->SetRadius( 0.f, 1.f );
	}

	// Target the object causing the damage.

	if( eFactType == kFact_Character )
	{
		TargetCharacter( pAI, pTargetFact );
	}
	else {
		TargetObject( pAI, pTargetFact );
	}

pAI->GetAIBlackBoard()->SetBBTargetLastVisibleTime( g_pLTServer->GetTime() );

}

