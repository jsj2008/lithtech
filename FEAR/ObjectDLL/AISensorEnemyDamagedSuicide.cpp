// ----------------------------------------------------------------------- //
//
// MODULE  : AISensorEnemyDamagedSuicide.cpp
//
// PURPOSE : AISensorEnemyDamagedSuicide class implementation
//
// CREATED : 11/01/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AISensorEnemyDamagedSuicide.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Sensor, CAISensorEnemyDamagedSuicide, kSensor_EnemyDamagedSuicide );


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorEnemyDamagedSuicide::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAISensorEnemyDamagedSuicide::CAISensorEnemyDamagedSuicide()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorEnemyDamagedSuicide::Save
//
//	PURPOSE:	Save the sensor
//
// ----------------------------------------------------------------------- //

void CAISensorEnemyDamagedSuicide::Save(ILTMessage_Write *pMsg)
{
	super::Save( pMsg );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorEnemyDamagedSuicide::Load
//
//	PURPOSE:	Load the sensor
//
// ----------------------------------------------------------------------- //

void CAISensorEnemyDamagedSuicide::Load(ILTMessage_Read *pMsg)
{
	super::Load( pMsg );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorEnemyDamagedSuicide::UpdateSensor
//
//	PURPOSE:	Return true if this sensor updated, and the SensorMgr
//              should wait to update others.
//
// ----------------------------------------------------------------------- //

bool CAISensorEnemyDamagedSuicide::UpdateSensor()
{
	if( !super::UpdateSensor() )
	{
		return false;
	}

	// AI is not targeting anyone.

	if( !m_pAI->HasTarget( kTarget_Character ) )
	{
		return false;
	}

	// Bail if no target.

	HOBJECT hTarget = m_pAI->GetAIBlackBoard()->GetBBTargetObject();
	CCharacter* pChar = (CCharacter*)g_pLTServer->HandleToObject( hTarget );
	if( !pChar )
	{
		return false;
	}

	// Bail if no destructible.

	CDestructible* pDestructible = pChar->GetDestructible();
	if( !pDestructible )
	{
		return false;
	}

	// Bail if we were not the last damager.

	if( pDestructible->GetLastDamager() != m_pAI->m_hObject )
	{
		return false;
	}

	// No new damage.

	if( pDestructible->GetLastDamageTime() <= 0.f )
	{
		return false;
	}

	// We damaged the target.
	// So kill myself.
	
	g_pCmdMgr->QueueMessage( m_pAI, m_pAI, "DESTROY" );

	return true;
}

