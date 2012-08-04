// ----------------------------------------------------------------------- //
//
// MODULE  : AISensorAbstract.h
//
// PURPOSE : AISensorAbstract abstract class definition
//
// CREATED : 1/28/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AISENSOR_ABSTRACT_H__
#define __AISENSOR_ABSTRACT_H__

#include "AIClassFactory.h"


//
// ENUM: Types of sensors.
//
enum EnumAISensorType
{
	kSensor_InvalidType= -1,
	#define SENSOR_TYPE_AS_ENUM 1
	#include "AIEnumSensorTypes.h"
	#undef SENSOR_TYPE_AS_ENUM

	kSensor_Count,
};

//
// STRINGS: const strings for sensor types.
//
static const char* s_aszSensorTypes[] =
{
	#define SENSOR_TYPE_AS_STRING 1
	#include "AIEnumSensorTypes.h"
	#undef SENSOR_TYPE_AS_STRING
};


// Forward declarations.

class	CAI;
class	CAIStimulusRecord;
struct	AIDB_SensorRecord;


// ----------------------------------------------------------------------- //

class CAISensorAbstract : public CAIClassAbstract
{
	public:
		static const char* const	GetSensorTypeName(EnumAISensorType eSensorType);
		static EnumAISensorType		GetSensorType(const char* const);

	public:
		DECLARE_AI_FACTORY_CLASS_ABSTRACT_SPECIFIC( Sensor );

		CAISensorAbstract( );
		virtual ~CAISensorAbstract( );

		virtual void	Save(ILTMessage_Write *pMsg);
		virtual void	Load(ILTMessage_Read *pMsg);

		virtual void	InitSensor( EnumAISensorType eSensorType, CAI* pAI );

		// Updating.

		float			GetSensorUpdateRate() const;
		double			GetNextSensorUpdateTime() const { return m_fNextSensorUpdateTime; }
		void			SetNextSensorUpdateTime( double fTime ) { m_fNextSensorUpdateTime = fTime; }

		// Handles updating the sensors state.  Return true if this sensor 
		// updated, and the SensorMgr should wait to update others.
		virtual bool	UpdateSensor();

		virtual bool	StimulateSensor( CAIStimulusRecord* pStimulusRecord );
		virtual void	DestimulateSensor() {}

		// LockCount.

		void			IncrementSensorRefCount() { ++m_cSensorRefCount; }
		void			DecrementSensorRefCount() { --m_cSensorRefCount; }
		int				GetSensorRefCount() const { return m_cSensorRefCount; }

		// Data Access.

		EnumAISensorType	GetSensorType() const { return m_eSensorType; }

	protected:

		EnumAISensorType		m_eSensorType;
		AIDB_SensorRecord*		m_pSensorRecord;	
		double					m_fNextSensorUpdateTime;
		int						m_cSensorRefCount;
		CAI*					m_pAI;
};


#endif
