// ----------------------------------------------------------------------- //
//
// MODULE  : AITargetSelectWeaponItem.cpp
//
// PURPOSE : 
//
// CREATED : 7/07/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AITargetSelectWeaponItem.h"
#include "AI.h"
#include "AIWorkingMemory.h"
#include "AIBlackBoard.h"
#include "AIQuadTree.h"
#include "AIPathMgrNavMesh.h"

LINKFROM_MODULE(AITargetSelectWeaponItem);

DEFINE_AI_FACTORY_CLASS_SPECIFIC( TargetSelect, CAITargetSelectWeaponItem, kTargetSelect_WeaponItem );

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAITargetSelectWeaponItem::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAITargetSelectWeaponItem::CAITargetSelectWeaponItem()
{
}

CAITargetSelectWeaponItem::~CAITargetSelectWeaponItem()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAITargetSelectWeaponItem::ValidateContextPreconditions
//
//	PURPOSE:	Return true if real-time preconditions are valid.
//
// ----------------------------------------------------------------------- //

bool CAITargetSelectWeaponItem::ValidatePreconditions( CAI* pAI )
{
	// Sanity check.

	if( !pAI )
	{
		return false;
	}

	// Must have targeted a character first, to avoid continuously resetting
	// the target as the AI attempts to acquire a character target.

	if ( !(pAI->GetAIBlackBoard()->GetBBTargetedTypeMask() & kTarget_Character) )
	{
		return false;
	}

	// Path is currently blocked by a player (we may want to expand this to anyone?).

	if ( 0 != ( ( kAIMovementFlag_BlockedPath | kAIMovementFlag_BlockedDestination ) & pAI->GetAIBlackBoard()->GetBBMovementCollisionFlags() ) )
	{
		CAIWMFact queryBlockedPathFact;
		queryBlockedPathFact.SetFactType( kFact_Knowledge );
		queryBlockedPathFact.SetKnowledgeType( kKnowledge_BlockedPath );
		CAIWMFact* pBlockedPathFact = pAI->GetAIWorkingMemory()->FindWMFact( queryBlockedPathFact );
		if ( pBlockedPathFact )
		{
			if ( IsPlayer( pBlockedPathFact->GetTargetObject() ) )
			{
				return false;
			}
		}
	}

	// AI has acquired a weapon and no longer needs a weapon

	if ( AIWeaponUtils::HasWeaponType(pAI, kAIWeaponType_Ranged, AIWEAP_CHECK_HOLSTER)
		|| AIWeaponUtils::HasWeaponType(pAI, kAIWeaponType_Melee, AIWEAP_CHECK_HOLSTER))
	{
		return false;
	}

	// Failed to find a fact about a usable weapon in the world.

	CAIWMFact factUsableQuery;
	factUsableQuery.SetFactType( kFact_Knowledge );
	factUsableQuery.SetKnowledgeType( kKnowledge_UsableWeaponItem ); 
	CAIWMFact* pUseableFact = pAI->GetAIWorkingMemory()->FindWMFact(factUsableQuery);
	if (!pUseableFact)
	{
		return false;
	}

	// No Target object (WeaponItem)

	if (!pUseableFact->GetTargetObject())
	{
		return false;
	}

	CAIWMFact queryFact;
	queryFact.SetFactType(kFact_Knowledge);
	queryFact.SetKnowledgeType(kKnowledge_WeaponItem);
	queryFact.SetTargetObject(pUseableFact->GetTargetObject());
	CAIWMFact* pWeaponItemFact = pAI->GetAIWorkingMemory()->FindWMFact(queryFact);

	// Failed to find a fact about this WeaponPickup.

	if (!pWeaponItemFact)
	{
		return false;
	}

	// Timed out.

	if (g_pLTServer->GetTime() < pWeaponItemFact->GetTime())
	{
		return false;
	}

	// Success!

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAITargetSelectWeaponItem::Activate
//
//	PURPOSE:	Activate selector.
//
// ----------------------------------------------------------------------- //

void CAITargetSelectWeaponItem::Activate( CAI* pAI )
{
	super::Activate( pAI );

	// Sanity check.

	if( ! pAI )
	{
		return;
	}
 
	CAIWMFact* pSelectedFact = NULL;

	// If the AI is already targetting a weapon, and if that weapon is 
	// valid, continue to target it.  This fixes the 'target re-selection 
	// causes indecisiveness' problem.

	if ( kTarget_WeaponItem == pAI->GetAIBlackBoard()->GetBBTargetType() 
		&& pAI->GetAIBlackBoard()->GetBBTargetObject() )
	{
		CAIWMFact queryFact;
		queryFact.SetFactType(kFact_Knowledge);
		queryFact.SetKnowledgeType(kKnowledge_WeaponItem);
		queryFact.SetTargetObject(pAI->GetAIBlackBoard()->GetBBTargetObject());
		CAIWMFact* pFact = pAI->GetAIWorkingMemory()->FindWMFact(queryFact);
		if ( pFact && g_pLTServer->GetTime() >= pFact->GetTime() )
		{
			pSelectedFact = pFact;
		}
	}

	// The current target isn't a valid weapon.  Find one; the existence of a valid 
	// weapon is guarenteed by the context precondition tests.

	if ( !pSelectedFact )
	{
		CAIWMFact factQuery;
		factQuery.SetFactType( kFact_Knowledge );
		factQuery.SetKnowledgeType( kKnowledge_UsableWeaponItem ); 
		pSelectedFact = pAI->GetAIWorkingMemory()->FindWMFact(factQuery);
	}

	// Failed to find a fact about a usable weapon in the world.

	if ( !pSelectedFact )
	{
		return;
	}

	// Record new target on the BlackBoard.

	pAI->GetAIBlackBoard()->SetBBTargetType( kTarget_WeaponItem );
	pAI->GetAIBlackBoard()->SetBBTargetObject( pSelectedFact->GetTargetObject() );
	pAI->GetAIBlackBoard()->SetBBTargetChangeTime( g_pLTServer->GetTime() );
	pAI->GetAIBlackBoard()->SetBBTargetStimulusID( kStimID_Invalid );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAITargetSelectWeaponItem::Validate
//
//	PURPOSE:	Returns true if AI should keep targeting the same target.
//
// ----------------------------------------------------------------------- //

bool CAITargetSelectWeaponItem::Validate( CAI* pAI )
{
	// WeaponPickup no longer exists.

	if (!pAI->GetAIBlackBoard()->GetBBTargetObject())
	{
		return false;
	}

	CAIWMFact queryFact;
	queryFact.SetFactType(kFact_Knowledge);
	queryFact.SetKnowledgeType(kKnowledge_WeaponItem);
	queryFact.SetTargetObject(pAI->GetAIBlackBoard()->GetBBTargetObject());
	CAIWMFact* pFact = pAI->GetAIWorkingMemory()->FindWMFact(queryFact);

	// AI has acquired a weapon and no longer needs a weapon

	if ( AIWeaponUtils::HasWeaponType(pAI, kAIWeaponType_Ranged, AIWEAP_CHECK_HOLSTER)
		|| AIWeaponUtils::HasWeaponType(pAI, kAIWeaponType_Melee, AIWEAP_CHECK_HOLSTER))
	{
		return false;
	}

	// Failed to find a fact about this WeaponPickup.

	if (!pFact)
	{
		return false;
	}

	// Timed out.

	if (g_pLTServer->GetTime() < pFact->GetTime())
	{
		return false;
	}

	// Path is currently blocked by a player (we may want to expand this to anyone?).
	// This is a workaround for the possibility that the AI is blocked, while
	// unarmed, by a player while running to the weaponitem.  This target selector 
	// needs to yield control so that the berserker selector can activate.

	if ( 0 != ( ( kAIMovementFlag_BlockedPath | kAIMovementFlag_BlockedDestination ) & pAI->GetAIBlackBoard()->GetBBMovementCollisionFlags() ) )
	{
		CAIWMFact queryBlockedPathFact;
		queryBlockedPathFact.SetFactType( kFact_Knowledge );
		queryBlockedPathFact.SetKnowledgeType( kKnowledge_BlockedPath );
		CAIWMFact* pBlockedPathFact = pAI->GetAIWorkingMemory()->FindWMFact( queryBlockedPathFact );
		if ( pBlockedPathFact )
		{
			if ( IsPlayer( pBlockedPathFact->GetTargetObject() ) )
			{
				return false;
			}
		}
	}

	return true;
}
