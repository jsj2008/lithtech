// ----------------------------------------------------------------------- //
//
// MODULE  : AISensorCoolMove.cpp
//
// PURPOSE : AISensorCoolMove class implementation
//
// CREATED : 05/07/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AISensorCoolMove.h"
#include "AI.h"
#include "AIBlackBoard.h"
#include "AITarget.h"
#include "AISoundMgr.h"
#include "ServerDB.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Sensor, CAISensorCoolMove, kSensor_CoolMove );

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorCoolMove::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAISensorCoolMove::CAISensorCoolMove()
{
	m_bEnteredSlowMo = false;
	m_fLastCoolMoveTime = 0.f;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorCoolMove::Save
//
//	PURPOSE:	Save the sensor
//
// ----------------------------------------------------------------------- //

void CAISensorCoolMove::Save(ILTMessage_Write *pMsg)
{
	super::Save( pMsg );

	SAVE_bool( m_bEnteredSlowMo );
	SAVE_TIME( m_fLastCoolMoveTime );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorCoolMove::Load
//
//	PURPOSE:	Load the sensor
//
// ----------------------------------------------------------------------- //

void CAISensorCoolMove::Load(ILTMessage_Read *pMsg)
{
	super::Load( pMsg );

	LOAD_bool( m_bEnteredSlowMo );
	LOAD_TIME( m_fLastCoolMoveTime );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorCoolMove::UpdateSensor
//
//	PURPOSE:	Return true if this sensor updated, and the SensorMgr
//              should wait to update others.
//
// ----------------------------------------------------------------------- //

bool CAISensorCoolMove::UpdateSensor()
{
	if( !super::UpdateSensor() )
	{
		return false;
	}

	// Bail if AI is not targeting a character.

	if( !m_pAI->HasTarget( kTarget_Character ) )
	{
		return false;
	}

	// Bail if AI cannot see his target.

	if( !m_pAI->GetAIBlackBoard()->GetBBTargetVisibleFromWeapon() )
	{
		return false;
	}


	//
	// Announce noticing the player entering SlowMo.
	//

	if( IsPlayer( m_pAI->GetAIBlackBoard()->GetBBTargetObject() ) )
	{
		// Irrelevant if Player is not using SlowMo.

		HRECORD hSlowMoCurrent = g_pGameServerShell->GetSlowMoRecord();
		if( hSlowMoCurrent )
		{
			// Only announce SlowMo when Player first enters SlowMo.

			if( !m_bEnteredSlowMo )
			{
				m_bEnteredSlowMo = true;

				// Only notice the default SlowMo (as opposed to cinematic, or cheats).

				HRECORD hSlowMoRequired = g_pServerDB->GetPlayerRecordLink( SrvDB_rSlowMo );
				if( hSlowMoCurrent == hSlowMoRequired )
				{
					g_pAISoundMgr->RequestAISound( m_pAI->m_hObject, kAIS_SlowMo, kAISndCat_InterruptMelee, NULL, 0.f );
					return false;
				}
			}
		}

		// Player is no longer using SlowMo.

		else {
			m_bEnteredSlowMo = false;
		}
	}


	//
	// Announce a "cool move" as identified by a model string on the anim.
	//

	// Bail if target does not exist.

	CCharacter* pChar = (CCharacter*)g_pLTServer->HandleToObject( m_pAI->GetAIBlackBoard()->GetBBTargetObject() );
	if( !pChar )
	{
		return false;
	}

	// Bail if no new cool move.

	double fCoolMoveTime = pChar->GetLastCoolMoveTime();
	if( fCoolMoveTime <= m_fLastCoolMoveTime )
	{
		return false;
	}

	// Speak if cool move was recent.

	if( ( g_pLTServer->GetTime() - fCoolMoveTime ) < 1.f )
	{
		LTVector vPos = m_pAI->GetAIBlackBoard()->GetBBTargetPosition();
		float fDistSqr = vPos.DistSqr( m_pAI->GetPosition() );

		if( fDistSqr < 1000.f * 1000.f )
		{
			g_pAISoundMgr->RequestAISound( m_pAI->m_hObject, kAIS_WhatThe, kAISndCat_InterruptMelee, NULL, 0.f );
		}
	}

	// Cache last cool move time.

	m_fLastCoolMoveTime = fCoolMoveTime;

	return true;
}
