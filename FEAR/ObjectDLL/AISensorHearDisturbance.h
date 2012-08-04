// ----------------------------------------------------------------------- //
//
// MODULE  : AISensorHearDisturbance.h
//
// PURPOSE : AISensorHearDisturbance class definition
//
// CREATED : 3/25/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AISENSOR_HEAR_DISTURBANCE_H__
#define __AISENSOR_HEAR_DISTURBANCE_H__

#include "AISensorAbstractStimulatable.h"


class CAISensorHearDisturbance : public CAISensorAbstractStimulatable
{
	typedef CAISensorAbstractStimulatable super;

	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( Sensor, CAISensorHearDisturbance, kSensor_HearDisturbance );

		CAISensorHearDisturbance();

		// CAISensorAbstract members.

	protected:

		bool				DidCharacterDisturbObject( CAIStimulusRecord* pStimulusRecord, HOBJECT* phChar, HOBJECT* phTarget );
		void				RecordLastDisturbanceSourceCharacter( HOBJECT hChar, HOBJECT hObject );
		bool				IgnoreDisturbanceBetweenObjects( CAIStimulusRecord* pStimulusRecord );
		bool				IgnoreDisturbanceFromObject( HOBJECT hObject );

		// CAISensorAbstractStimulatable members.

		virtual float		GetSenseDistSqr( float fStimulusRadius );
		virtual bool		StimulateSensor( CAIStimulusRecord* pStimulusRecord );
		virtual CAIWMFact*	CreateWorkingMemoryFact( CAIStimulusRecord* pStimulusRecord );
		virtual float		IncreaseStimulation( float fCurStimulation, float fRateModifier, CAIStimulusRecord* pStimulusRecord, AIDB_StimulusRecord* pStimulus );
		virtual void		FindUnstimulatedWorkingMemoryFact(AIWORKING_MEMORY_FACT_LIST* pOutFactList);
};

#endif
