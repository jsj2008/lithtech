// ----------------------------------------------------------------------- //
//
// MODULE  : AITargetSelectTraitor.cpp
//
// PURPOSE : 
//
// CREATED : 3/16/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

// TODO: We may need to invalidate the target if no one is attempting to 
// engage the player(?), or should this be handled by the sensor?

#include "Stdafx.h"
#include "AITargetSelectTraitor.h"
#include "PlayerObj.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( TargetSelect, CAITargetSelectTraitor, kTargetSelect_Traitor );

// This handles finding the 'best' traitor to attack.
struct TraitorCollector
{
	TraitorCollector() : 
		m_pBestTraitor(NULL)
		, m_flBestConfidence( 0.0f )
	{
	}

	void operator()( CAIWMFact* pFact )
	{
		if ( kFact_Character != pFact->GetFactType()
			|| ( 0 == ( kFactFlag_CharacterIsTraitor & pFact->GetFactFlags() ) ) )
		{
			return;
		}

		float flConfidence = pFact->GetConfidence( CAIWMFact::kFactMask_Stimulus );
		if ( flConfidence <= m_flBestConfidence )
		{
			return;
		}

		m_pBestTraitor		= pFact;
		m_flBestConfidence	= flConfidence;
	}

	CAIWMFact*	m_pBestTraitor;
	float		m_flBestConfidence;
};

static bool AllPlayersTargeted( CAI* pIgnoreThisAI )
{
	CPlayerObj::PlayerObjList::const_iterator itEachPlayer = CPlayerObj::GetPlayerObjList().begin();
	CPlayerObj::PlayerObjList::const_iterator itEndPlayer = CPlayerObj::GetPlayerObjList().end();
	for ( ; itEachPlayer != itEndPlayer; ++itEachPlayer )
	{
		// Ignore this player if they are not valid or if they are not alive.

		CCharacter* pPlayer = *itEachPlayer;
		if ( NULL == pPlayer 
			|| IsDeadCharacter( pPlayer->GetHOBJECT() ) )
		{
			continue;
		}
		HOBJECT hPlayer = pPlayer->GetHOBJECT();

		// See if any AIs are targeting the player.

		bool bPlayerTargeted = false;
		CAI::AIList::const_iterator itEachAI = CAI::GetAIList().begin();
		CAI::AIList::const_iterator itEndAI = CAI::GetAIList().end();
		for (  ; itEachAI != itEndAI; ++itEachAI )
		{
			CAI* pCurrentAI = *itEachAI;
			if ( NULL == pCurrentAI )
			{
				continue;
			}

			if ( pCurrentAI == pIgnoreThisAI )
			{
				continue;
			}

			if ( pCurrentAI->GetAIBlackBoard()->GetBBTargetObject() != hPlayer )
			{
				continue;
			}

			bPlayerTargeted = true;
			break;
		}

		if ( false == bPlayerTargeted )
		{
			// Found an untargeted player.  Not all players are targeted.

			return false;
		}
	}

	// All players are targeted.

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAITargetSelectTraitor::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAITargetSelectTraitor::CAITargetSelectTraitor()
{
}

CAITargetSelectTraitor::~CAITargetSelectTraitor()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAITargetSelectTraitor::ValidateContextPreconditions
//
//	PURPOSE:	Return true if real-time preconditions are valid.
//
// ----------------------------------------------------------------------- //

bool CAITargetSelectTraitor::ValidatePreconditions( CAI* pAI )
{
	// Sanity check.

	if( !pAI )
	{
		return false;
	}

	// Fail if no one is targeting one of the players.

	if ( false == AllPlayersTargeted( pAI ) )
	{
		return false;
	}

	// Fail if there is not a valid traitor target.

	TraitorCollector BestTraitor;
	pAI->GetAIWorkingMemory()->CollectFact( BestTraitor );
	CAIWMFact* pTraitorFact = BestTraitor.m_pBestTraitor;
	if ( NULL == pTraitorFact )
	{
		return false;
	}

	// Found a valid target.

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAITargetSelectTraitor::Activate
//
//	PURPOSE:	Activate selector.
//
// ----------------------------------------------------------------------- //

void CAITargetSelectTraitor::Activate( CAI* pAI )
{
	super::Activate( pAI );

	// Sanity check.

	if( !pAI )
	{
		return;
	}

	// Find the target again.

	TraitorCollector BestTraitor;
	pAI->GetAIWorkingMemory()->CollectFact( BestTraitor );
	CAIWMFact* pTraitorFact = BestTraitor.m_pBestTraitor;
	AIASSERT( pTraitorFact, pAI->GetHOBJECT(), "CAITargetSelectTraitor::Activate" );
	if ( NULL == pTraitorFact )
	{
		return;
	}

	HOBJECT hTraitor = pTraitorFact->GetTargetObject();

	// Update the target.

	pAI->GetAIBlackBoard()->SetBBTargetType( kTarget_Character );
	pAI->GetAIBlackBoard()->SetBBTargetStimulusType( kStim_InvalidType );
	pAI->GetAIBlackBoard()->SetBBTargetStimulusID( kStimID_Invalid );
	pAI->GetAIBlackBoard()->SetBBTargetChangeTime( g_pLTServer->GetTime() );
	pAI->GetAIBlackBoard()->SetBBTargetObject( hTraitor );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAITargetSelectTraitor::Validate
//
//	PURPOSE:	Returns true if AI should keep targeting the same target.
//
// ----------------------------------------------------------------------- //

bool CAITargetSelectTraitor::Validate( CAI* pAI )
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

	// No one is targeting one of the players.

	if ( false == AllPlayersTargeted( pAI ) )
	{
		return false;
	}

	// Target is still valid.

	return true;
}
