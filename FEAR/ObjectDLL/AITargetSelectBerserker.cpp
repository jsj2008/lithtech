// ----------------------------------------------------------------------- //
//
// MODULE  : AITargetSelectBerserker.cpp
//
// PURPOSE : This target selection sets the AI's target to the 
//			 object the AI should perform a berserk attack against.  
//
//			 This selector is required as the AI should consider a 
//			 berserker attack higher priority than picking up a weapon,
//			 which is itself higher priority than the character 
//			 selectors
//
//			 Additionally, the AI needs to chase the player when the 
//			 player is close by, but only when they are unarmed.  The
//			 commitment to start this process occurs here, at this 
//			 level.
//
// CREATED : 8/11/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AITargetSelectBerserker.h"
#include "AI.h"
#include "AIBlackBoard.h"

LINKFROM_MODULE(AITargetSelectBerserker);

DEFINE_AI_FACTORY_CLASS_SPECIFIC( TargetSelect, CAITargetSelectBerserker, kTargetSelect_Berserker );

class BerserkerDesire
{
public:
	BerserkerDesire( CAI* pAI ) : 
		m_pSelectedFact( NULL )
		, m_pAI( pAI )
	{
	}

public:
	void operator()( CAIWMFact* pFact )
	{
		// Bail if not a desire
		// Bail if not a berserker desire
		// Bail if timed out
		if ( kFact_Desire != pFact->GetFactType()
			|| kDesire_Berserker != pFact->GetDesireType()
			|| g_pLTServer->GetTime() < pFact->GetTime() )
		{
			return;
		}

		// Success!

		m_pSelectedFact = pFact;
	}

	CAIWMFact*	m_pSelectedFact;
	CAI*		m_pAI;
};


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAITargetSelectBerserker::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAITargetSelectBerserker::CAITargetSelectBerserker()
{
}

CAITargetSelectBerserker::~CAITargetSelectBerserker()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAITargetSelectBerserker::ValidateContextPreconditions
//
//	PURPOSE:	Return true if real-time preconditions are valid.
//
// ----------------------------------------------------------------------- //

bool CAITargetSelectBerserker::ValidatePreconditions( CAI* pAI )
{
	if( !super::ValidatePreconditions( pAI ) )
	{
		return false;
	}

	// Sanity check.

	if( !pAI )
	{
		return false;
	}

	// AI has acquired a weapon and no longer needs to berserker attack.

	if ( AIWeaponUtils::HasWeaponType(pAI, kAIWeaponType_Ranged, AIWEAP_CHECK_HOLSTER)
		|| AIWeaponUtils::HasWeaponType(pAI, kAIWeaponType_Melee, AIWEAP_CHECK_HOLSTER))
	{
		return false;
	}

	// Fail if the AI does not have a desire to perform a berserker attack 
	// that is not timed out.

	BerserkerDesire BerserkerTarget( pAI );
	pAI->GetAIWorkingMemory()->CollectFact( BerserkerTarget );
	if ( !BerserkerTarget.m_pSelectedFact )
	{
		return false;
	}

	// Found a target the AI desires to perform a berserker attack against.

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAITargetSelectBerserker::Activate
//
//	PURPOSE:	Activate selector.
//
// ----------------------------------------------------------------------- //

void CAITargetSelectBerserker::Activate( CAI* pAI )
{
	super::Activate( pAI );

	// Sanity check.

	if( !pAI )
	{
		return;
	}

	// Bail if AI does not have a target

	BerserkerDesire BerserkerTarget( pAI );
	pAI->GetAIWorkingMemory()->CollectFact( BerserkerTarget );

	CAIWMFact* pFact = BerserkerTarget.m_pSelectedFact;
	if( !pFact )
	{
		return;
	}

	HOBJECT hTarget = pFact->GetTargetObject();

	// Record new target on the BlackBoard.

	pAI->GetAIBlackBoard()->SetBBTargetType( kTarget_Berserker );
	pAI->GetAIBlackBoard()->SetBBTargetChangeTime( g_pLTServer->GetTime() );
	pAI->GetAIBlackBoard()->SetBBTargetObject( hTarget );

	AIASSERT( IsPlayer( hTarget ), pAI->m_hObject, "CAITargetSelectCharacter::TargetCharacter: factObject is not a player" );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAITargetSelectBerserker::Validate
//
//	PURPOSE:	Returns true if AI should keep targeting the same target.
//
// ----------------------------------------------------------------------- //

bool CAITargetSelectBerserker::Validate( CAI* pAI )
{
	if ( !super::Validate( pAI ) )
	{
		return false;
	}

	// Target is dead.

	if( !pAI->GetAIBlackBoard()->GetBBTargetObject() )
	{
		return false;
	}

	// Timed out.

	CAIWMFact factQuery;
	factQuery.SetFactType( kFact_Desire );
	factQuery.SetDesireType( kDesire_Berserker );
	factQuery.SetTargetObject( pAI->GetAIBlackBoard()->GetBBTargetObject() );
	CAIWMFact* pBerserkerDesire = pAI->GetAIWorkingMemory()->FindWMFact( factQuery );
	if ( !pBerserkerDesire || g_pLTServer->GetTime() < pBerserkerDesire->GetTime() )
	{
		return false;
	}

	// AI has acquired a weapon.

	if ( AIWeaponUtils::HasWeaponType(pAI, kAIWeaponType_Ranged, AIWEAP_CHECK_HOLSTER)
		|| AIWeaponUtils::HasWeaponType(pAI, kAIWeaponType_Melee, AIWEAP_CHECK_HOLSTER))
	{
		return false;
	}

	// Target is still valid.

	return true;
}
