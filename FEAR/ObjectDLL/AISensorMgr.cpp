// ----------------------------------------------------------------------- //
//
// MODULE  : AISensorMgr.cpp
//
// PURPOSE : AISensorMgr abstract class implementation
//
// CREATED : 2/03/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AISensorMgr.h"
#include "AISensorAbstract.h"
#include "AI.h"
#include "AIDB.h"
#include "AIStimulusMgr.h"
#include "iperformancemonitor.h"

DEFINE_AI_FACTORY_CLASS( CAISensorMgr );

// Performance monitoring.
CTimedSystem g_tsAISensors("AISensors", "AI");


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorMgr::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAISensorMgr::CAISensorMgr()
{
	m_pAI = NULL;

	m_fStimulusListNewIterationTime = 0.f;
	m_bDoneProcessingStimuli = true;
	m_iSensorToUpdate = 0;
	m_bSensorDeleted = false;
}

CAISensorMgr::~CAISensorMgr()
{
	TermAISensorMgr();
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAISensorMgr::Save/Load
//              
//	PURPOSE:	Handle saving and restoring the CAISensorMgr and all of its 
//				sensors.
//              
//----------------------------------------------------------------------------
void CAISensorMgr::Save(ILTMessage_Write *pMsg)
{
	SAVE_COBJECT(m_pAI);

	SAVE_INT(m_lstAISensors.size());
	{for (std::size_t n = 0; n < m_lstAISensors.size(); ++n)
	{
		SAVE_INT(m_lstAISensors[n]->GetSensorClassType());
		m_lstAISensors[n]->Save(pMsg);
	}}

	SAVE_bool(m_bSensorDeleted);
	SAVE_INT(m_iSensorToUpdate);
	SAVE_bool(m_bDoneProcessingStimuli);
	SAVE_TIME( m_fStimulusListNewIterationTime );

	SAVE_INT(m_lstProcessedStimuli.size());
	{for (std::size_t n = 0; n < m_lstProcessedStimuli.size(); ++n)
	{
		SAVE_INT(m_lstProcessedStimuli.size());
	}}

	SAVE_INT(m_cIntersectSegmentCount);
}

void CAISensorMgr::Load(ILTMessage_Read *pMsg)
{
	LOAD_COBJECT(m_pAI, CAI);

	int nSensorCount = 0;
	LOAD_INT(nSensorCount);
	{for (int n = 0; n < nSensorCount; ++n)
	{
		EnumAISensorType eSensor;
		LOAD_INT_CAST(eSensor, EnumAISensorType);
		CAISensorAbstract* pSensor = AI_FACTORY_NEW_Sensor( eSensor );
		pSensor->Load(pMsg);

		m_lstAISensors.push_back(pSensor);
	}}

	LOAD_bool(m_bSensorDeleted);
	LOAD_INT(m_iSensorToUpdate);
	LOAD_bool(m_bDoneProcessingStimuli);
	LOAD_TIME( m_fStimulusListNewIterationTime );

	int nProcessedStimuli = 0;
	LOAD_INT(nProcessedStimuli);
	{for (int n = 0; n < nProcessedStimuli; ++n)
	{
		EnumAIStimulusID eStimulusID;
		LOAD_INT_CAST(eStimulusID, EnumAIStimulusID);
		m_lstProcessedStimuli.push_back(eStimulusID);
	}}

	LOAD_INT(m_cIntersectSegmentCount);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorMgr::InitSensorMgr
//
//	PURPOSE:	Initialize AISensorMgr.
//
// ----------------------------------------------------------------------- //

void CAISensorMgr::InitSensorMgr( CAI* pAI )
{
	m_pAI = pAI;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorMgr::TermAISensorMgr
//
//	PURPOSE:	Terminate AISensorMgr.
//
// ----------------------------------------------------------------------- //

void CAISensorMgr::TermAISensorMgr()
{
	// Delete instances of sensors.

	AISENSOR_LIST::iterator itSensor;
	for( itSensor = m_lstAISensors.begin(); itSensor != m_lstAISensors.end(); ++itSensor )
	{
		AI_FACTORY_DELETE( *itSensor );
	}
	m_lstAISensors.resize( 0 );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorMgr::AddAISensor
//
//	PURPOSE:	Add an AISensor to the AISensorMgr.
//
// ----------------------------------------------------------------------- //

CAISensorAbstract* CAISensorMgr::AddAISensor( EnumAISensorType eSensorType )
{
	// Find an existing sensor of the specified type.

	CAISensorAbstract* pSensor = FindSensor( eSensorType );
	if( pSensor )
	{
		// Incremenet the reference count.

		pSensor->IncrementSensorRefCount();
		return pSensor;
	}

	// Create a new sensor of the specified type.

	AIDB_SensorRecord* pSensorRecord;
	pSensorRecord = g_pAIDB->GetAISensorRecord( eSensorType );

	if( pSensorRecord && ( pSensorRecord->eSensorClass != kSensor_InvalidType ) )
	{
		pSensor = AI_FACTORY_NEW_Sensor( pSensorRecord->eSensorClass );
		if (pSensor)
		{
			m_lstAISensors.push_back( pSensor );
			pSensor->InitSensor( eSensorType, m_pAI );
		}
	}

	return pSensor;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorMgr::RemoveAISensor
//
//	PURPOSE:	Remove an AISensor from the AISensorMgr.
//
// ----------------------------------------------------------------------- //

void CAISensorMgr::RemoveAISensor( EnumAISensorType eSensorType )
{
	// Find a sensor of the specified type.

	CAISensorAbstract* pSensor = FindSensor( eSensorType );
	if( pSensor )
	{
		// Decrement the reference count of the sensor.

		pSensor->DecrementSensorRefCount();
		if( pSensor->GetSensorRefCount() <= 0 )
		{
			m_bSensorDeleted = true;
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorMgr::DeleteAISensor
//
//	PURPOSE:	Delete an AISensor.
//
// ----------------------------------------------------------------------- //

AISENSOR_LIST::iterator CAISensorMgr::DeleteAISensor( EnumAISensorType eSensorType )
{
	// Delete instance of a sensor.

	CAISensorAbstract* pSensor;
	AISENSOR_LIST::iterator itSensor;
	for( itSensor = m_lstAISensors.begin(); itSensor != m_lstAISensors.end(); ++itSensor )
	{
		pSensor = *itSensor;
		if( pSensor->GetSensorType() == eSensorType )
		{
			AI_FACTORY_DELETE( pSensor );
			return m_lstAISensors.erase( itSensor );
		}
	}

	return m_lstAISensors.end();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorMgr::AddAISensor
//
//	PURPOSE:	Return a pointer to a sensor of the specified type, if it exists.
//
// ----------------------------------------------------------------------- //

CAISensorAbstract* CAISensorMgr::FindSensor( EnumAISensorType eSensorType )
{
	CAISensorAbstract* pSensor;
	AISENSOR_LIST::iterator itSensor;
	for( itSensor = m_lstAISensors.begin(); itSensor != m_lstAISensors.end(); ++itSensor )
	{
		pSensor = *itSensor;
		if( pSensor->GetSensorType() == eSensorType )
		{
			return pSensor;
		}
	}

	return NULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorMgr::AI_FACTORY_NEW_Sensor
//
//	PURPOSE:	Create a sensor.
//
// ----------------------------------------------------------------------- //

CAISensorAbstract* CAISensorMgr::AI_FACTORY_NEW_Sensor( EnumAISensorType eSensorType )
{
	// Call AI_FACTORY_NEW for the requested type of sensor.

	switch( eSensorType )
	{
		#define SENSOR_TYPE_AS_SWITCH 1
		#include "AIEnumSensorTypes.h"
		#undef SENSOR_TYPE_AS_SWITCH

		default: AIASSERT( 0, NULL, "CAISensorMgr::AI_FACTORY_NEW_Sensor: Unrecognized sensor type." );
	}

	return NULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorMgr::UpdateSensors
//
//	PURPOSE:	Return true if a sensor that does not update every frame
//              has done a significant amount of work.
//
// ----------------------------------------------------------------------- //

bool CAISensorMgr::UpdateSensors( bool bUpdateDistributedSensors )
{
	//track our performance
	CTimedSystemBlock TimingBlock(g_tsAISensors);

	// Delete sensors that have a zero lock count.

	CAISensorAbstract* pSensor;
	AISENSOR_LIST::iterator itSensor;
	if( m_bSensorDeleted )
	{
		itSensor = m_lstAISensors.begin();
		while( itSensor != m_lstAISensors.end() )
		{
			pSensor = *itSensor;
			if( pSensor->GetSensorRefCount() <= 0 )
			{
				itSensor = DeleteAISensor( pSensor->GetSensorType() );
			}
			else {
				++itSensor;
			}
		}
		m_bSensorDeleted = false;
	}

	// Update remaining sensors.

	double fTime = g_pLTServer->GetTime();
	float fUpdateRate;

	int cSensors = m_lstAISensors.size();
	if( cSensors == 0 )
	{
		return false;
	}

	// Bound the sensor index, in case sensors were removed.

	int iFirstToUpdate = m_iSensorToUpdate;
	if( iFirstToUpdate >= cSensors )
	{
		m_iSensorToUpdate = 0;
		iFirstToUpdate = 0;
	}

	// Iterate over sensors until one is updated, or all have been checked.

	bool bUpdated = false;
	int iSensorToUpdate = iFirstToUpdate;
	while( bUpdateDistributedSensors )
	{
		// Get sensor at some index, and increment the index.

		pSensor = m_lstAISensors[iSensorToUpdate];
		iSensorToUpdate = ( iSensorToUpdate + 1 ) % cSensors;

		// Update all sensors that have an update rate of 0.0 every update.

		fUpdateRate = pSensor->GetSensorUpdateRate();
		if( fUpdateRate == 0.f ) 
		{
			pSensor->UpdateSensor();
		}

		// Update the sensor if it is time, and no other distributed sensors have updated.

		else if( ( !bUpdated ) &&
				 ( pSensor->GetNextSensorUpdateTime() <= fTime ) )
		{
			bUpdated = pSensor->UpdateSensor();
			pSensor->SetNextSensorUpdateTime( fTime + fUpdateRate );
			if( bUpdated )
			{
				m_iSensorToUpdate = iSensorToUpdate;
			}
		}

		// Bail if index has wrapped.

		if( iSensorToUpdate == iFirstToUpdate )
		{
			break;
		}
	}

	return bUpdated;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorMgr::StimulateSensors
//
//	PURPOSE:	Stimulate sensors.
//
// ----------------------------------------------------------------------- //

bool CAISensorMgr::StimulateSensors( CAIStimulusRecord* pStimulusRecord )
{
	bool bStimulated = false;
	CAISensorAbstract* pSensor;
	AISENSOR_LIST::iterator itSensor;
	for( itSensor = m_lstAISensors.begin(); itSensor != m_lstAISensors.end(); ++itSensor )
	{
		pSensor = *itSensor;
		if( pSensor->StimulateSensor( pStimulusRecord ) )
		{
			bStimulated = true;
		}
	}	

	return bStimulated;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorMgr::HandleSenses
//
//	PURPOSE:	Handle stimulatable sensors that were not stimulated this stimulus update.
//
// ----------------------------------------------------------------------- //

void CAISensorMgr::HandleSenses( uint32 nCycle )
{
	CAISensorAbstract* pSensor;
	AISENSOR_LIST::iterator itSensor;
	for( itSensor = m_lstAISensors.begin(); itSensor != m_lstAISensors.end(); ++itSensor )
	{
		pSensor = *itSensor;
		pSensor->DestimulateSensor();
	}	
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorMgr::*ProcessingStimuli()
//
//	PURPOSE:	Process stimuli.
//
// ----------------------------------------------------------------------- //

void CAISensorMgr::ClearProcessedStimuli()
{
	m_lstProcessedStimuli.resize( 0 );
	m_fStimulusListNewIterationTime = g_pLTServer->GetTime();
}

bool CAISensorMgr::ProcessStimulus( CAIStimulusRecord* pRecord )
{
	AI_PROCESSED_STIMULI_LIST::iterator itStim;
	for( itStim = m_lstProcessedStimuli.begin(); itStim != m_lstProcessedStimuli.end(); ++itStim )
	{
		if( *itStim == pRecord->m_eStimulusID )
		{
			return false;
		}
	}

	m_lstProcessedStimuli.push_back( pRecord->m_eStimulusID );
	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorMgr::*IntersectSegmentCount()
//
//	PURPOSE:	Handle IntersectSegment Counting.
//
// ----------------------------------------------------------------------- //

int	CAISensorMgr::GetIntersectSegmentCount() const
{
	return m_cIntersectSegmentCount;
}

void CAISensorMgr::ClearIntersectSegmentCount()
{
	m_cIntersectSegmentCount = 0;
}

void CAISensorMgr::IncrementIntersectSegmentCount()
{
	++m_cIntersectSegmentCount;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorMgr::SenseToString/SenseFromString
//
//	PURPOSE:	Convert a sense enum to a string and viceversa.
//
// ----------------------------------------------------------------------- //

const char* CAISensorMgr::SenseToString(EnumAISenseType eSenseType)
{
	for( uint32 iSense=0; iSense < kSense_Count; ++iSense )
	{
		if( eSenseType & (1 << iSense) )
		{
			return s_aszSenseTypes[iSense];
		}
	}

	return NULL;
}

EnumAISenseType CAISensorMgr::SenseFromString(char* szSenseType)
{
	for(uint32 iSenseType = 0; iSenseType < kSense_Count; ++iSenseType)
	{
		if( LTStrICmp(szSenseType, s_aszSenseTypes[iSenseType]) == 0 )
		{
			return (EnumAISenseType)(1 << iSenseType);
		}
	}

	return kSense_InvalidType;
}
