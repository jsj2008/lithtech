// ----------------------------------------------------------------------- //
//
// MODULE  : AITargetSelectPlayerTurret.cpp
//
// PURPOSE : AITargetSelectPlayerTurret class definition
//
// CREATED : 8/02/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AITargetSelectPlayerTurret.h"
#include "AI.h"
#include "AIBlackBoard.h"
#include "AIWorkingMemory.h"
#include "AISoundMgr.h"
#include "PlayerObj.h"
#include "Turret.h"
#include "CharacterDB.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( TargetSelect, CAITargetSelectPlayerTurret, kTargetSelect_PlayerTurret );


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAITargetSelectPlayerTurret::ValidateContextPreconditions
//
//	PURPOSE:	Return true if real-time preconditions are valid.
//
// ----------------------------------------------------------------------- //

bool CAITargetSelectPlayerTurret::ValidatePreconditions( CAI* pAI )
{
	// Intentionally do NOT call super::ValidateContextPreconditions.
	// This Selector is only AI knows of a player turret.

	// Sanity check.

	if( !pAI )
	{
		return false;
	}

	// Fail if AI is not aware of a player turret.

	bool bTurretExists = false;
	CAIWMFact* pFact = pAI->GetAIWorkingMemory()->FindFactPlayerTurret( pAI );
	if( pFact )
	{
		bTurretExists = true;
	}

	// Find a disturbance caused by a turret.

	if( !bTurretExists )
	{
		// Bail if no disturbances.

		pFact = pAI->GetAIWorkingMemory()->FindFactDisturbanceMax();
		if( !pFact )
		{
			return false;
		}

		// Bail if disturbance was not recent.

		if( pFact->GetUpdateTime() < g_pLTServer->GetTime() - 1.f )
		{
			return false;
		}

		// Target disturbance, or target an allie's target.

		HOBJECT hTarget = pFact->GetTargetObject();
		if( IsAI( hTarget ) )
		{
			CAI* pTargetAI = (CAI*)g_pLTServer->HandleToObject( hTarget );
			if( pTargetAI && 
				( g_pCharacterDB->GetStance( pAI->GetAlignment(), pTargetAI->GetAlignment() ) != kCharStance_Hate ) )
			{
				hTarget = pTargetAI->GetAIBlackBoard()->GetBBTargetObject();
			}
		}

		// Bail if disturbance was not caused by a player.

		if( !IsPlayer( hTarget ) )
		{
			return false;
		}

		// Bail if Player is not using a turret.

		CPlayerObj* pPlayer = (CPlayerObj*)g_pLTServer->HandleToObject( hTarget );
		if( !( pPlayer && pPlayer->GetTurret() ) )
		{
			return false;
		}

		// Disturbance was caused by a turret.

		bTurretExists = true;
	}

	// Preconditions are met.

	return bTurretExists;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAITargetSelectPlayerTurret::Activate
//
//	PURPOSE:	Activate selector.
//
// ----------------------------------------------------------------------- //

void CAITargetSelectPlayerTurret::Activate( CAI* pAI )
{
	// Intentionally do NOT call super::Activate.
	// Target a player turret.
	//
	// Be sure to call down to the base class however to insure that any 
	// awareness modifications are applied.

	CAITargetSelectAbstract::Activate( pAI );

	// Sanity check.

	if( !pAI )
	{
		return;
	}

	// AI is aware of a turret.

	HOBJECT hTarget = NULL;
	CAIWMFact* pFact = pAI->GetAIWorkingMemory()->FindFactPlayerTurret( pAI );
	if( pFact )
	{
		hTarget = pFact->GetTargetObject();
	}

	// AI is aware of a disturbance caused by a turret.

	if( !hTarget )
	{
		// Bail if no disturbances.

		pFact = pAI->GetAIWorkingMemory()->FindFactDisturbanceMax();
		if( !pFact )
		{
			return;
		}

		// Bail if disturbance was not caused by a player.

		HOBJECT hDisturbanceTarget = pFact->GetTargetObject();
		if( !IsPlayer( hDisturbanceTarget ) )
		{
			return;
		}

		// Bail if Player is not using a turret.

		CPlayerObj* pPlayer = (CPlayerObj*)g_pLTServer->HandleToObject( hDisturbanceTarget );
		if( !( pPlayer && pPlayer->GetTurret() ) )
		{
			return;
		}

		// Target is the turret.

		hTarget = pPlayer->GetTurret();
	}

	// Bail if the AI is already targeting this object.

	if( hTarget == pAI->GetAIBlackBoard()->GetBBTargetObject() 
		&& ( kTarget_Object == pAI->GetAIBlackBoard()->GetBBTargetType() ) )
	{
		return;
	}

	
	// Create a working memory fact for this turret.

	CAIWMFact* pTargetFact = pAI->GetAIWorkingMemory()->CreateWMFact( kFact_Object );
	pTargetFact->SetTargetObject( hTarget, 1.f );

	EnumAIStimulusID eStimulusID;
	EnumAIStimulusType eStimulusType;
	pFact->GetStimulus( &eStimulusType, &eStimulusID );
	pTargetFact->SetStimulus( eStimulusType, eStimulusID, 0.f );
	pTargetFact->SetPos( pFact->GetPos(), 1.f );
	pTargetFact->SetRadius( 0.f, 1.f );

	// Announce turret.

	g_pAISoundMgr->RequestAISound( pAI->m_hObject, kAIS_TurretDetected, kAISndCat_Event, hTarget, 0.f );

	// Ensure we can track the turret.

	uint32 dwFlags = pAI->GetAIBlackBoard()->GetBBTargetPosTrackingFlags();
	if( dwFlags & kTargetTrack_Squad )
	{
		dwFlags = dwFlags & ~kTargetTrack_Squad;
		dwFlags |= kTargetTrack_Normal;
		pAI->GetAIBlackBoard()->SetBBTargetPosTrackingFlags( dwFlags );
	}

	// Target the object.

	TargetObject( pAI, pTargetFact );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAITargetSelectPlayerTurret::Validate
//
//	PURPOSE:	Returns true if AI should keep targeting the same target.
//
// ----------------------------------------------------------------------- //

bool CAITargetSelectPlayerTurret::Validate( CAI* pAI )
{
	// Sanity check.

	if( !pAI )
	{
		return false;
	}

	// Bail if turret no longer exists.

	HOBJECT hTarget = pAI->GetAIBlackBoard()->GetBBTargetObject();
	if( !hTarget )
	{
		return false;
	}

	// Bail if the turret is no longer in use.

	if( IsTurret( hTarget ) )
	{
		Turret* pTurret = (Turret*)g_pLTServer->HandleToObject( hTarget );
		if( !pTurret->IsInUse() )
		{
			return false;
		}
	}

	return super::Validate( pAI );
}
