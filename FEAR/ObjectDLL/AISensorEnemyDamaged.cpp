// ----------------------------------------------------------------------- //
//
// MODULE  : AISensorEnemyDamaged.cpp
//
// PURPOSE : AISensorEnemyDamaged class implementation
//
// CREATED : 04/23/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AISensorEnemyDamaged.h"
#include "AI.h"
#include "AITarget.h"
#include "AIBlackBoard.h"
#include "AIWorkingMemory.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Sensor, CAISensorEnemyDamaged, kSensor_EnemyDamaged );


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorEnemyDamaged::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAISensorEnemyDamaged::CAISensorEnemyDamaged()
{
	m_hTarget = NULL;
	m_fLastDamageTime = 0.f;
	m_nHitsBeforeRetreat = 0;
	m_nMaxHitsBeforeRetreat = 2;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorEnemyDamaged::Save
//
//	PURPOSE:	Save the sensor
//
// ----------------------------------------------------------------------- //

void CAISensorEnemyDamaged::Save(ILTMessage_Write *pMsg)
{
	super::Save( pMsg );

	SAVE_HOBJECT( m_hTarget );
	SAVE_TIME( m_fLastDamageTime );
	SAVE_INT( m_nHitsBeforeRetreat );
	SAVE_INT( m_nMaxHitsBeforeRetreat );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorEnemyDamaged::Load
//
//	PURPOSE:	Load the sensor
//
// ----------------------------------------------------------------------- //

void CAISensorEnemyDamaged::Load(ILTMessage_Read *pMsg)
{
	super::Load( pMsg );

	LOAD_HOBJECT( m_hTarget );
	LOAD_TIME( m_fLastDamageTime );
	LOAD_INT( m_nHitsBeforeRetreat );
	LOAD_INT( m_nMaxHitsBeforeRetreat );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorEnemyDamaged::InitSensor
//
//	PURPOSE:	Initialize the sensor.
//
// ----------------------------------------------------------------------- //

void CAISensorEnemyDamaged::InitSensor( EnumAISensorType eSensorType, CAI* pAI )
{
	super::InitSensor( eSensorType, pAI );

	m_nHitsBeforeRetreat = GetRandom( 1, m_nMaxHitsBeforeRetreat );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorEnemyDamaged::UpdateSensor
//
//	PURPOSE:	Return true if this sensor updated, and the SensorMgr
//              should wait to update others.
//
// ----------------------------------------------------------------------- //

bool CAISensorEnemyDamaged::UpdateSensor()
{
	if( !super::UpdateSensor() )
	{
		return false;
	}

	// AI is not targeting anyone.

	if( !m_pAI->HasTarget( kTarget_Character ) )
	{
		m_hTarget = NULL;
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

	// Target changed.

	if( m_hTarget != hTarget )
	{
		m_hTarget = hTarget;
		m_fLastDamageTime = pDestructible->GetLastDamageTime();
		return false;
	}

	// Bail if we were not the last damager.

	if( pDestructible->GetLastDamager() != m_pAI->m_hObject )
	{
		return false;
	}

	// No new damage.

	double fLastDamageTime = pDestructible->GetLastDamageTime();
	if( fLastDamageTime <= m_fLastDamageTime )
	{
		return false;
	}

	// We damaged the target.

	float fLastDamage = pDestructible->GetLastDamage();
	float fLastArmorAbsorb = pDestructible->GetLastArmorAbsorb();

	if( ( fLastArmorAbsorb > 0.f ) ||
		( fLastDamage > 0.f ) )
	{
		// Decrement required hit count.

		--m_nHitsBeforeRetreat;
		if( m_nHitsBeforeRetreat <= 0 )
		{
			// Find an existing memory for the desire to retreat, or create one.

			CAIWMFact factQuery;
			factQuery.SetFactType(kFact_Desire);
			factQuery.SetDesireType(kDesire_Retreat);
			CAIWMFact* pFact = m_pAI->GetAIWorkingMemory()->FindWMFact( factQuery );
			if( !pFact )
			{
				pFact = m_pAI->GetAIWorkingMemory()->CreateWMFact( kFact_Desire );
				pFact->SetDesireType( kDesire_Retreat, 1.f );
			}

			m_nHitsBeforeRetreat = GetRandom( 1, m_nMaxHitsBeforeRetreat );
		}
	}

	// Update the last damage time.

	m_fLastDamageTime = fLastDamageTime;

	return true;
}

