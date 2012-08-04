// ----------------------------------------------------------------------- //
//
// MODULE  : AITargetSelectDisturbanceBeyondGuard.cpp
//
// PURPOSE : AITargetSelectDisturbanceBeyondGuard class definition
//
// CREATED : 6/22/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AITargetSelectDisturbanceBeyondGuard.h"
#include "AI.h"
#include "AITarget.h"
#include "AINodeGuard.h"
#include "AIBlackBoard.h"
#include "AIWorkingMemory.h"
#include "AIUtils.h"
#include "CharacterDB.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( TargetSelect, CAITargetSelectDisturbanceBeyondGuard, kTargetSelect_DisturbanceBeyondGuard );


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAITargetSelectDisturbanceBeyondGuard::Constructor
//
//	PURPOSE:	Constructor.
//
// ----------------------------------------------------------------------- //

CAITargetSelectDisturbanceBeyondGuard::CAITargetSelectDisturbanceBeyondGuard()
{
	m_bRecordFirstThreat = false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAITargetSelectDisturbanceBeyondGuard::ValidateContextPreconditions
//
//	PURPOSE:	Return true if real-time preconditions are valid.
//
// ----------------------------------------------------------------------- //

bool CAITargetSelectDisturbanceBeyondGuard::ValidatePreconditions( CAI* pAI )
{
	// Intentionally do NOT call super::ValidateContextPreconditions.
	// This Selector is only valid if AI is reacting to a disturbance outside his guard area.

	// Sanity check.

	if( !pAI )
	{
		return false;
	}

	// Fail if AI was previously aware of a character target.

	if( pAI->GetAIBlackBoard()->GetBBTargetedTypeMask() & kTarget_Character )
	{
		return false;
	}

	// No disturbance.

	CAIWMFact* pFact = pAI->GetAIWorkingMemory()->FindFactDisturbanceMax();
	if( !pFact )
	{
		return false;
	}

	// Disturbance is not from a character.

	HOBJECT hTarget = pFact->GetTargetObject();
	if( !IsCharacter( hTarget ) )
	{
		return false;
	}

	// Find AI's Guard node.

	CAIWMFact factGuardQuery;
	AINodeGuard* pNodeGuard = NULL;
	factGuardQuery.SetFactType(kFact_Node);
	factGuardQuery.SetNodeType(kNode_Guard);
	pFact = pAI->GetAIWorkingMemory()->FindWMFact( factGuardQuery );
	if( pFact && IsAINode( pFact->GetTargetObject() ) )
	{
		HOBJECT hNode = pFact->GetTargetObject();
		AINode* pNode = (AINode*)g_pLTServer->HandleToObject( hNode );
		if( pNode->GetType() == kNode_Guard )
		{
			pNodeGuard = (AINodeGuard*)pNode;
		}
	}

	// No Guard node.

	if( !pNodeGuard )
	{
		return false;
	}

	// Selector is only valid if disturbance is outside of the Guarded area.

	if( pNodeGuard->IsCharacterInRadiusOrRegion( hTarget ) )
	{
		return false;
	}

	// Preconditions are met.

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAITargetSelectDisturbanceBeyondGuard::Activate
//
//	PURPOSE:	Activate selector.
//
// ----------------------------------------------------------------------- //

void CAITargetSelectDisturbanceBeyondGuard::Activate( CAI* pAI )
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

	// Either target the disturbance directly, or if the disturbance 
	// is coming from an Ally, target ally's target.

	HOBJECT hTarget = SelectDisturbanceSource( pAI, pFact );

	// Bail if the AI is already targeting this character.

	if( hTarget == pAI->GetAIBlackBoard()->GetBBTargetObject() 
		&& kTarget_Character == pAI->GetAIBlackBoard()->GetBBTargetType() )
	{
		return;
	}

	// Find or create a working memory fact for this character.

	CAIWMFact factQuery;
	factQuery.SetFactType( kFact_Character );
	factQuery.SetTargetObject( hTarget );
	CAIWMFact* pTargetFact = pAI->GetAIWorkingMemory()->FindWMFact( factQuery );
	if( !pTargetFact )
	{
		pTargetFact = pAI->GetAIWorkingMemory()->CreateWMFact( kFact_Character );
		pTargetFact->SetTargetObject( hTarget, 1.f );

		EnumAIStimulusID eStimulusID;
		EnumAIStimulusType eStimulusType;
		pFact->GetStimulus( &eStimulusType, &eStimulusID );
		pTargetFact->SetStimulus( eStimulusType, eStimulusID, 0.f );
		pTargetFact->SetPos( pFact->GetPos(), 1.f );
		pTargetFact->SetRadius( 0.f, 1.f );
		pTargetFact->SetFactFlags( 0 );
	}

	// Target character causing the disturbance.

	TargetCharacter( pAI, pTargetFact );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAITargetSelectDisturbanceBeyondGuard::SelectDisturbanceSource
//
//	PURPOSE:	Activate selector.
//
// ----------------------------------------------------------------------- //

HOBJECT CAITargetSelectDisturbanceBeyondGuard::SelectDisturbanceSource( CAI* pAI, CAIWMFact* pFact )
{
	// Sanity check.

	if( !( pAI && pFact ) )
	{
		return NULL;
	}

	// Disturbance is not from an AI.

	if( !IsAI( pFact->GetTargetObject() ) )
	{
		return pFact->GetTargetObject();
	}

	// AI is not an ally.

	CAI* pOther = (CAI*)g_pLTServer->HandleToObject( pFact->GetTargetObject() );
	EnumCharacterStance eStance = g_pCharacterDB->GetStance( pAI->GetAlignment(), pOther->GetAlignment() );
	if( eStance != kCharStance_Like )
	{
		return pFact->GetTargetObject();
	}

	// Ally is not targeting a character.

	if( pOther->GetAIBlackBoard()->GetBBTargetType() != kTarget_Character )
	{
		return pFact->GetTargetObject();
	}

	// Return the Ally's target object.

	return pOther->GetAIBlackBoard()->GetBBTargetObject();
}
