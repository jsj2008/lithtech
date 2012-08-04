// ----------------------------------------------------------------------- //
//
// MODULE  : AISensorMgr.h
//
// PURPOSE : AISensorMgr abstract class definition
//
// CREATED : 2/03/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AISENSOR_MGR_H__
#define __AISENSOR_MGR_H__

#include "AIClassFactory.h"
#include "AISensorAbstract.h"
#include "AIEnumStimulusTypes.h"

// Forward declarations.

class	CAI;
class	CAISensorAbstract;
class	CAIStimulusRecord;

//
// ENUM: Types of senses exclusive bitflags.
//
enum EnumAISenseType
{
	kSense_InvalidType	= 0,
	kSense_None			= 0,

	#define SENSE_TYPE_AS_FLAG 1
	#include "AISenseTypeEnums.h"
	#undef SENSE_TYPE_AS_FLAG

	kSense_All			= 0xffffffff,
};

// Expand as enums to automatically get the kSense_Count.  The enum values
// here are not used.
enum
{
	#define SENSE_TYPE_AS_ENUM 1
	#include "AISenseTypeEnums.h"
	#undef SENSE_TYPE_AS_ENUM
	kSense_Count
};

static const char* s_aszSenseTypes[] =
{
	#define SENSE_TYPE_AS_STRING 1
	#include "AISenseTypeEnums.h"
	#undef SENSE_TYPE_AS_STRING
};


typedef std::vector< CAISensorAbstract*, LTAllocator<CAISensorAbstract*, LT_MEM_TYPE_OBJECTSHELL> >	AISENSOR_LIST;

typedef std::vector<EnumAIStimulusID, LTAllocator<EnumAIStimulusID, LT_MEM_TYPE_OBJECTSHELL> > AI_PROCESSED_STIMULI_LIST;

// ----------------------------------------------------------------------- //

class CAISensorMgr : public CAIClassAbstract
{
	public:
		DECLARE_AI_FACTORY_CLASS( CAISensorMgr );

		CAISensorMgr();
		~CAISensorMgr();

		void	Save(ILTMessage_Write *pMsg);
		void	Load(ILTMessage_Read *pMsg);

		void	InitSensorMgr( CAI* pAI );
		void	TermAISensorMgr();

		CAISensorAbstract*		AddAISensor( EnumAISensorType eSensorType );
		void					RemoveAISensor( EnumAISensorType eSensorType );
		AISENSOR_LIST::iterator	DeleteAISensor( EnumAISensorType eSensorType );
		CAISensorAbstract*		FindSensor( EnumAISensorType eSensorType );

		bool	UpdateSensors( bool bUpdateDistributedSensors );
		bool	StimulateSensors( CAIStimulusRecord* pStimulusRecord );

		double			GetStimulusListNewIterationTime() const { return m_fStimulusListNewIterationTime; }
		virtual bool	GetDoneProcessingStimuli() { return m_bDoneProcessingStimuli; }
		virtual void	SetDoneProcessingStimuli( bool bDone ) { m_bDoneProcessingStimuli = bDone; }
		virtual void	ClearProcessedStimuli();
		virtual bool	ProcessStimulus( CAIStimulusRecord* pRecord );

		virtual int		GetIntersectSegmentCount() const;
		virtual void	ClearIntersectSegmentCount();
		virtual void	IncrementIntersectSegmentCount();

		virtual void	HandleSenses(uint32 nCycle);

		// Static methods

		static const char* SenseToString( EnumAISenseType eSenseType );
		static EnumAISenseType SenseFromString( char* szSenseType );

	protected:

		CAISensorAbstract*	AI_FACTORY_NEW_Sensor( EnumAISensorType eSensorType );

	protected:

		CAI*			m_pAI;
		AISENSOR_LIST	m_lstAISensors;
		int				m_iSensorToUpdate;
		bool			m_bSensorDeleted;

		double						m_fStimulusListNewIterationTime;
		bool						m_bDoneProcessingStimuli;
		AI_PROCESSED_STIMULI_LIST	m_lstProcessedStimuli;

		int							m_cIntersectSegmentCount;
};

// ----------------------------------------------------------------------- //

#endif
