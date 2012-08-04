// ----------------------------------------------------------------------- //
//
// MODULE  : AISensorSeeDisturbance.h
//
// PURPOSE : AISensorSeeDisturbance class definition
//
// CREATED : 3/31/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AISENSOR_SEE_DISTURBANCE_H__
#define __AISENSOR_SEE_DISTURBANCE_H__

#include "AISensorAbstractStimulatable.h"


class CAISensorSeeDisturbance : public CAISensorAbstractStimulatable
{
	typedef CAISensorAbstractStimulatable super;

	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( Sensor, CAISensorSeeDisturbance, kSensor_SeeDisturbance );

		CAISensorSeeDisturbance();

		// CAISensorAbstract members.

	protected:

		// CAISensorAbstractStimulatable members.

		virtual float		GetSenseDistSqr( float fStimulusRadius );
		virtual bool		StimulateSensor( CAIStimulusRecord* pStimulusRecord );
		virtual bool		DoComplexCheck( CAIStimulusRecord* pStimulusRecord, float* pfRateModifier );
		virtual CAIWMFact*	CreateWorkingMemoryFact( CAIStimulusRecord* pStimulusRecord );
		virtual float		IncreaseStimulation( float fCurStimulation, float fRateModifier, CAIStimulusRecord* pStimulusRecord, AIDB_StimulusRecord* pStimulus );
};

#endif
