// ----------------------------------------------------------------------- //
//
// MODULE  : AITargetSelectScripted.cpp
//
// PURPOSE : AITargetSelectScripted class definition
//
// CREATED : 7/01/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AITargetSelectScripted.h"
#include "AI.h"
#include "AIBlackBoard.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( TargetSelect, CAITargetSelectScripted, kTargetSelect_Scripted );


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAITargetSelectScripted::ValidateContextPreconditions
//
//	PURPOSE:	Return true if real-time preconditions are valid.
//
// ----------------------------------------------------------------------- //

bool CAITargetSelectScripted::ValidatePreconditions( CAI* pAI )
{
	// Intentionally do NOT call super::ValidateContextPreconditions.
	// Target is solely based on the blackboard value set via a command.

	// Sanity check.

	if( !pAI )
	{
		return false;
	}

	// Fail if AI does not have a scripted target.

	if( !pAI->GetAIBlackBoard()->GetBBScriptedTargetObject() )
	{
		return false;
	}

	// Preconditions are met.

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAITargetSelectScripted::Activate
//
//	PURPOSE:	Activate selector.
//
// ----------------------------------------------------------------------- //

void CAITargetSelectScripted::Activate( CAI* pAI )
{
	// Intentionally do NOT call super::Activate.
	// Target is solely based on the blackboard value set via a command.
	//
	// Be sure to call down to the base class however to insure that any 
	// awareness modifications are applied.
	CAITargetSelectAbstract::Activate( pAI );

	// Sanity check.

	if( !pAI )
	{
		return;
	}

	// Fail if AI does not have a scripted target.

	HOBJECT hScriptedTarget = pAI->GetAIBlackBoard()->GetBBScriptedTargetObject();
	if( !hScriptedTarget )
	{
		return;
	}

	// Bail if the AI is already targeting this object.

	if( hScriptedTarget == pAI->GetAIBlackBoard()->GetBBTargetObject() 
		&& ( kTarget_Character == pAI->GetAIBlackBoard()->GetBBTargetType() || kTarget_Object == pAI->GetAIBlackBoard()->GetBBTargetType() ) )
	{
		return;
	}

	// Scripted target may be a character, or something else.

	ENUM_AIWMFACT_TYPE eFactType = IsCharacter( hScriptedTarget ) ? kFact_Character : kFact_Object;

	// Find or create a working memory fact for this target object.

	CAIWMFact factQuery;
	factQuery.SetFactType( eFactType );
	factQuery.SetTargetObject( hScriptedTarget );
	CAIWMFact* pFact = pAI->GetAIWorkingMemory()->FindWMFact( factQuery );
	if( !pFact )
	{
		pFact = pAI->GetAIWorkingMemory()->CreateWMFact( eFactType );
		pFact->SetTargetObject( hScriptedTarget, 1.f );

		pFact->SetStimulus( kStim_InvalidType, kStimID_Invalid, 0.f );

		LTVector vPos;
		g_pLTServer->GetObjectPos( hScriptedTarget, &vPos );
		pFact->SetPos( vPos, 1.f );
		pFact->SetRadius( 0.f, 1.f );
		pFact->SetFactFlags( 0 );
	}

	// Target the object.

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
//	ROUTINE:	CAITargetSelectScripted::Validate
//
//	PURPOSE:	Returns true if AI should keep targeting the same target.
//
// ----------------------------------------------------------------------- //

bool CAITargetSelectScripted::Validate( CAI* pAI )
{
	// Intentionally do NOT call super::Validate.
	// We do not care of the target is out of view.

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

	// Target is still valid.

	return true;
}
