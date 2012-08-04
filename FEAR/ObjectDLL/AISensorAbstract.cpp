// ----------------------------------------------------------------------- //
//
// MODULE  : AISensorAbstract.cpp
//
// PURPOSE : AISensorAbstract abstract class implementation
//
// CREATED : 1/28/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AISensorAbstract.h"
#include "AIDB.h"

// Insures the AI do not initially all update their sensors the same frame.
static double s_fSensorUpdateBasis = 0.0f;

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CAISensorAbstract::GetSensorTypeName
//
//  PURPOSE:	Static function which returns the name of a the passed in 
//				sensor.
//
// ----------------------------------------------------------------------- //

const char* const CAISensorAbstract::GetSensorTypeName(EnumAISensorType eSensorType)
{
	if (eSensorType < 0 || eSensorType >= kSensor_Count)
	{
		return "";
	}

	return s_aszSensorTypes[eSensorType];
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CAISensorAbstract::GetSensorType
//
//  PURPOSE:	Static function get the EnumAISensorType associated with 
//				the passed in sensor name string.
//
// ----------------------------------------------------------------------- //

EnumAISensorType CAISensorAbstract::GetSensorType(const char* const pszSensorTypeName)
{
	if (NULL == pszSensorTypeName)
	{
		return kSensor_InvalidType;
	}

	for (int i = 0; i < kSensor_Count; ++i)
	{
		if (LTStrIEquals(s_aszSensorTypes[i], pszSensorTypeName))
		{
			return (EnumAISensorType)i;
		}
	}

	return kSensor_InvalidType;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorAbstract::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAISensorAbstract::CAISensorAbstract()
{
	m_eSensorType = kSensor_InvalidType;
	m_pSensorRecord = NULL;
	m_fNextSensorUpdateTime = 0.f;
	m_cSensorRefCount = 0;
	m_pAI = NULL;
}

CAISensorAbstract::~CAISensorAbstract()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorAbstract::Save
//
//	PURPOSE:	Save the sensor
//
// ----------------------------------------------------------------------- //

void CAISensorAbstract::Save(ILTMessage_Write *pMsg)
{
	SAVE_INT(m_eSensorType);
	SAVE_DOUBLE(m_fNextSensorUpdateTime);
	SAVE_INT(m_cSensorRefCount);
	SAVE_COBJECT(m_pAI);

	// Don't save, manually restore:
	// m_pSensorTemplate
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorAbstract::Load
//
//	PURPOSE:	Load the sensor
//
// ----------------------------------------------------------------------- //

void CAISensorAbstract::Load(ILTMessage_Read *pMsg)
{
	LOAD_INT_CAST(m_eSensorType, EnumAISensorType);
	LOAD_DOUBLE(m_fNextSensorUpdateTime);
	LOAD_INT(m_cSensorRefCount);
	LOAD_COBJECT(m_pAI, CAI);

	m_pSensorRecord = g_pAIDB->GetAISensorRecord( m_eSensorType );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorAbstract::InitSensor
//
//	PURPOSE:	Initialize the sensor.
//
// ----------------------------------------------------------------------- //

void CAISensorAbstract::InitSensor( EnumAISensorType eSensorType, CAI* pAI )
{
	m_pAI = pAI;
	m_eSensorType = eSensorType;
	m_pSensorRecord = g_pAIDB->GetAISensorRecord( m_eSensorType );
	AIASSERT( m_pSensorRecord, NULL, "CAISensorAbstract::InitSensor: Could not find template." );
	m_cSensorRefCount = 1;

	m_fNextSensorUpdateTime = g_pLTServer->GetTime() + s_fSensorUpdateBasis;
	s_fSensorUpdateBasis += .02f;
	if ( s_fSensorUpdateBasis > 1.f )
	{
		s_fSensorUpdateBasis = 0.0f;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorAbstract::GetSensorUpdateRate
//
//	PURPOSE:	Return rate (in secs) sensor updates.
//              Zero means sensor updates every frame.
//
// ----------------------------------------------------------------------- //

float CAISensorAbstract::GetSensorUpdateRate() const
{
	if( m_pSensorRecord )
	{
		return m_pSensorRecord->fSensorUpdateRate;
	}

	return 0.f;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorAbstract::UpdateSensor
//
//	PURPOSE:	Return true if this sensor updated, and the SensorMgr
//              should wait to update others.
//
// ----------------------------------------------------------------------- //

bool CAISensorAbstract::UpdateSensor()
{
	// Sanity checks.

	if( !m_pAI )
	{
		AIASSERT( 0, NULL, "CAISensorAbstract::UpdateSensor: AI is NULL." );
		return false;
	}

	if( !m_pSensorRecord )
	{
		AIASSERT( 0, NULL, "CAISensorAbstract::UpdateSensor: SensorRecord is NULL." );
		return false;
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorAbstract::StimulateSensor.
//
//	PURPOSE:	Stimulate the sensor.
//
// ----------------------------------------------------------------------- //

bool CAISensorAbstract::StimulateSensor( CAIStimulusRecord* pStimulusRecord )
{
	return false;
}

