// ----------------------------------------------------------------------- //
//
// MODULE  : AISensorDamageTurret.cpp
//
// PURPOSE : AISensorDamageTurret class implementation
//
// CREATED : 7/30/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AISensorDamageTurret.h"
#include "AI.h"
#include "AIBlackBoard.h"
#include "AIStimulusMgr.h"
#include "PlayerObj.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Sensor, CAISensorDamageTurret, kSensor_DamageTurret );


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorDamageTurret::IsDamagerTarget
//
//	PURPOSE:	Return true if the damager is already the current target.
//
// ----------------------------------------------------------------------- //

bool CAISensorDamageTurret::IsDamagerTarget( HOBJECT hDamager )
{
	// Intentionally do NOT call super::IsDamagerTarget.
	// Consider the damager the turret if a player is firing from a turret.

	if( IsPlayer( hDamager ) )
	{
		CPlayerObj* pPlayer = (CPlayerObj*)g_pLTServer->HandleToObject( hDamager );
		if( pPlayer && pPlayer->GetTurret() )
		{
			hDamager = pPlayer->GetTurret();
		}
	}

	return ( hDamager == m_pAI->GetAIBlackBoard()->GetBBTargetObject() );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorDamageTurret::SetFactTargetObject
//
//	PURPOSE:	Set the target object the WMFact.
//
// ----------------------------------------------------------------------- //

void CAISensorDamageTurret::SetFactTargetObject( CAIWMFact* pFact, CAIStimulusRecord* pStimulusRecord )
{
	// Intentionally do NOT call super::SetFactTargetObject.
	// Target the stimulus target rather than the source.
	// Consider the damager the turret if a player is firing from a turret.

	// Sanity check.

	if( !( pFact && pStimulusRecord ) )
	{
		return;
	}

	// Who shot me.

	HOBJECT hDamager = pStimulusRecord->m_hStimulusTarget;
	if( IsPlayer( hDamager ) )
	{
		CPlayerObj* pPlayer = (CPlayerObj*)g_pLTServer->HandleToObject( hDamager );
		if( pPlayer && pPlayer->GetTurret() )
		{
			hDamager = pPlayer->GetTurret();
		}
	}

	pFact->SetTargetObject( hDamager, 1.f );
}
