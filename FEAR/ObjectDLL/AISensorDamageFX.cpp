// ----------------------------------------------------------------------- //
//
// MODULE  : AISensorDamageFX.cpp
//
// PURPOSE : AISensorDamageFX class implementation
//
// CREATED : 02/22/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AISensorDamageFX.h"
#include "AIStimulusMgr.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Sensor, CAISensorDamageFX, kSensor_DamageFX );

#define DAMAGE_FX_75	"DamageFX_75"
#define DAMAGE_FX_50	"DamageFX_50"
#define DAMAGE_FX_25	"DamageFX_25"


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorDamageFX::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAISensorDamageFX::CAISensorDamageFX()
{
	m_bDamageFXInitialized = false;
	m_fFullHealth = 0.f;
	m_eDamageFXState = kDamageFX_None;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAISensorDamageFX::Save/Load
//              
//	PURPOSE:	Handle saving and restoring the CAISensorDamage
//
//----------------------------------------------------------------------------

void CAISensorDamageFX::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);

	SAVE_bool( m_bDamageFXInitialized );
	SAVE_FLOAT( m_fFullHealth );
	SAVE_DWORD( m_eDamageFXState );
}

void CAISensorDamageFX::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);

	LOAD_bool( m_bDamageFXInitialized );
	LOAD_FLOAT( m_fFullHealth );
	LOAD_DWORD_CAST( m_eDamageFXState, EnumAIDamageFXState );

	// Create FX that should exist after a load.

	CreateDamageFX( m_eDamageFXState );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorDamageFX::UpdateSensor
//
//	PURPOSE:	Return true if this sensor updated, and the SensorMgr
//              should wait to update others.
//
// ----------------------------------------------------------------------- //

bool CAISensorDamageFX::UpdateSensor()
{
	// Intentionally skip super::UpdateSensor.
	// This sensor doesn't handle delayed stimuli.

	if( !CAISensorAbstract::UpdateSensor() )
	{
		return false;
	}

	// Initialize the full health member.

	if( !m_bDamageFXInitialized )
	{
		m_fFullHealth = m_pAI->GetDestructible()->GetHitPoints();
		m_bDamageFXInitialized = true;
	}

	// Always allow other sensors to update.

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorDamageFX::StimulateSensor.
//
//	PURPOSE:	Stimulate the sensor.
//
// ----------------------------------------------------------------------- //

bool CAISensorDamageFX::StimulateSensor( CAIStimulusRecord* pStimulusRecord )
{
	// Intentionally do not call super::StimulateSensor.
	// Just monitor the AI's health and create FX.  Do not affect WorkingMemory.

	// Bail if the wrong type of stimulus.

	if( !( m_pSensorRecord->dwStimulusTypes & pStimulusRecord->m_eStimulusType ) )
	{
		return false;
	}

	// Sanity check.
	
	CDestructible* pDestructible = m_pAI->GetDestructible();
	if( ( m_fFullHealth <= 0.f ) || ( !pDestructible ) )
	{
		return false;
	}

	// Create damage FX based on the percentage of remaining health.

	float fHealthPercent = pDestructible->GetHitPoints() / m_fFullHealth;
	switch( m_eDamageFXState )
	{
		case kDamageFX_None:
			if( fHealthPercent < 0.75f )
			{
				m_eDamageFXState = kDamageFX_75;
				CreateDamageFX( kDamageFX_75 );
			}
			break;

		case kDamageFX_75:
			if( fHealthPercent < 0.5f )
			{
				m_eDamageFXState = kDamageFX_50;
				CreateDamageFX( kDamageFX_None );
				CreateDamageFX( kDamageFX_50 );
			}
			break;

		case kDamageFX_50:
			if( fHealthPercent < 0.25f )
			{
				m_eDamageFXState = kDamageFX_25;
				CreateDamageFX( kDamageFX_None );
				CreateDamageFX( kDamageFX_25 );
			}
			break;
	}

	// Always return false, to let other sensors handle damage.

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorDamageFX::CreateDamageFX
//
//	PURPOSE:	Create FX indicating AI is damaged.
//
// ----------------------------------------------------------------------- //

void CAISensorDamageFX::CreateDamageFX( EnumAIDamageFXState eDamageFXState )
{
	CAutoMessage cMsg;
	cMsg.Writeuint8( MID_SFX_MESSAGE );
	cMsg.Writeuint8( SFX_CHARACTER_ID );
	cMsg.WriteObject( m_pAI->m_hObject );

	// Kill looping FX, or create looping FX corresponding to level of health.

	const char* pszFX;
	std::string strFX = m_pAI->GetAIAttributes()->strName;
	switch( eDamageFXState )
	{
		case kDamageFX_None:
			cMsg.WriteBits(CFX_KILL_LOOP_FX_MSG, FNumBitsExclusive<CFX_COUNT>::k_nValue );
			break;

		case kDamageFX_75:
			strFX += DAMAGE_FX_75;
			pszFX = g_pAIDB->GetMiscString( strFX.c_str() );
			cMsg.WriteBits(CFX_CREATE_LOOP_FX_MSG, FNumBitsExclusive<CFX_COUNT>::k_nValue );
			cMsg.WriteString( pszFX );
			break;

		case kDamageFX_50:
			strFX += DAMAGE_FX_50;
			pszFX = g_pAIDB->GetMiscString( strFX.c_str() );
			cMsg.WriteBits(CFX_CREATE_LOOP_FX_MSG, FNumBitsExclusive<CFX_COUNT>::k_nValue );
			cMsg.WriteString( pszFX );
			break;

		case kDamageFX_25:
			strFX += DAMAGE_FX_25;
			pszFX = g_pAIDB->GetMiscString( strFX.c_str() );
			cMsg.WriteBits(CFX_CREATE_LOOP_FX_MSG, FNumBitsExclusive<CFX_COUNT>::k_nValue );
			cMsg.WriteString( pszFX );
			break;
	}

	g_pLTServer->SendToClient(cMsg.Read(), NULL, MESSAGE_GUARANTEED);
}
